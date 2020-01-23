var MotionControl = {};
MotionControl.GStack = [];
MotionControl.machine_parameters = {
	machine_extents: { x: 45.5, y: 45.5 },
	machine_axis_invert: { x: false, y1: false, y2: false, z: false },
	machine_axis_scale: { x: 518, y: 518, z: 2540, a: 21.85 },
	machine_max_vel: { x: 1500, y: 1500, z: 80, a: 30000 },
	machine_max_accel: { x: 60, y: 60, z: 60, a: 600 },
	machine_thc: { adc_filter: 20, tolerance: 3 },
	machine_junction_deviation: 0.010,
	machine_torch_config: { z_probe_feed: 60, floating_head_takeup: 0.200, clearance_height: 3 },
	work_offset: {x: 0, y: 0},
  };
MotionControl.way_point = null;
MotionControl.is_connected = false;
MotionControl.dro_data = { X_MCS: 0.0000, Y_MCS: 0.0000, X_WCS: 0.0000, Y_WCS: 0.0000, VELOCITY: 0.0, THC_ARC_VOLTAGE: 0.0, THC_SET_VOLTAGE: 0.0, STATUS: "Halt"};
MotionControl.thc_command = "Idle";
MotionControl.on_idle = null;
MotionControl.on_hold = null;
MotionControl.arc_ok = false;
MotionControl.mode = "XY";
MotionControl.adc_readings = [];
MotionControl.average_adc = function(val)
{
	this.adc_readings.push(val);
	if (this.adc_readings.length > this.machine_parameters.machine_thc.adc_filter)
	{
		this.adc_readings.shift();
	}
	var sum_total = 0;
	for (var x = 0; x < this.adc_readings.length; x++)
	{
		sum_total += this.adc_readings[x];
	}
	return (sum_total / this.adc_readings.length)
}

MotionControl.SaveParameters = function()
{
	if (file.open("machine_parameters.json", "w"))
	{
		file.write(JSON.stringify(this.machine_parameters));
		motion_control.set_work_offset({x: this.machine_parameters.work_offset.x, y:  this.machine_parameters.work_offset.y});
		file.close();
	}
}
MotionControl.PullParameters = function()
{
	var contents = file.get_contents("machine_parameters.json");
	//console.log("<" + contents + ">\n");
	if (contents != undefined)
	{
		this.machine_parameters = JSON.parse(contents);
		motion_control.set_work_offset({x: this.machine_parameters.work_offset.x, y:  this.machine_parameters.work_offset.y});
	}
}
MotionControl.ProgramAbort = function()
{
	motion_control.clear_stack();
	motion_control.abort();
}
MotionControl.set_waypoint = function(p)
{
	this.way_point = p;
}
MotionControl.go_to_waypoint = function()
{
	if (this.is_connected)
	{
		if (this.way_point != null)
		{
			//console.log("Going to waypoint!\n");
			this.send("G53 G0 X" + this.way_point.x.toFixed(4) + " Y" + this.way_point.y.toFixed(4));
			//this.send("G54");
		}
	}
}
MotionControl.init = function()
{
	console.log(JSON.stringify(serial.list_ports()) + "\n");
	this.PullParameters();
	motion_control.set_parameters(this.machine_parameters);
	motion_control.set_baud(115200);
	if (system.os() == "WINDOWS")
	{
		motion_control.set_port("USB");
		console.log("Setting connect port description query to: USB\n");
	}
	else
	{
		motion_control.set_port("Arduino");
		console.log("Setting connect port description query to: Arduino\n");
	}
	motion_control.set_dro_interval(125);
}
MotionControl.send_gcode_from_viewer = function()
{
	if (GcodeViewer.last_file != null)
	{
		//console.log("Running!\n");
		if (file.open(GcodeViewer.last_file, "r"))
		{
			//console.log("Opened: " + GcodeViewer.last_file + "\n");
			while(file.lines_available())
			{
				var line = file.read();
				//console.log("Sending line: \"" + line + "\"\n");
				if (line.includes("#"))
				{
					//Comment, don't do anything
				}
				else if (line.includes("G0") || line.includes("G1") || line.includes("torch"))
				{
					if (line.includes("fire_torch"))
					{
						//fire_torch 0.1200 1.2 0.0750
						var fire_torch_parameters = { pierce_height: 0.120, pierce_delay: 1.2, cut_height: 0.075 };
						var fire_torch = line.split(" ");
						for (var x = 0; x < fire_torch.length; x++)
						{
							if (x == 1) fire_torch_parameters.pierce_height = fire_torch[x];
							if (x == 2) fire_torch_parameters.pierce_delay = fire_torch[x];
							if (x == 3) fire_torch_parameters.cut_height = fire_torch[x];
						}
						MotionControl.send("G38.3 Z-10 F" + MotionControl.machine_parameters.machine_torch_config.z_probe_feed);
						MotionControl.send("G91 G0 Z" + this.machine_parameters.machine_torch_config.floating_head_takeup);
						MotionControl.send("G91 G0 Z" + fire_torch_parameters.pierce_height);
						MotionControl.send("M3 S1000");
						MotionControl.send("G4 P" + fire_torch_parameters.pierce_delay); //Pierce Delay
						MotionControl.send("G91 G0 Z" + (fire_torch_parameters.cut_height - fire_torch_parameters.pierce_height));
						MotionControl.send("G90"); //Back to absolute
					}
					else if (line.includes("torch_off"))
					{
						MotionControl.send("M5");
						MotionControl.send("G4 P1"); //Post Delay
						MotionControl.send("G91 G0 Z" + this.machine_parameters.machine_torch_config.clearance_height); //Retract
						MotionControl.send("G90"); //Back to absolute
					}
					else
					{
						MotionControl.send(line);
					}
				}
			}
			file.close();
		}
		else
		{
			//console.log("Could not read file!\n");
		}		//MotionControl.send("G53 G
	}
}
MotionControl.send_gcode_from_list = function(list)
{
	for (var z = 0; z < list.length; z++)
	{
		var line = list[z];
		//console.log("Sending line: " + line + "\n");
		if (line.includes("#"))
		{
			//Comment, don't do anything
		}
		else if (line.includes("G0") || line.includes("G1") || line.includes("torch"))
		{
			if (line.includes("fire_torch"))
			{
				//fire_torch 0.1200 1.2 0.0750
				var fire_torch_parameters = { pierce_height: 0.120, pierce_delay: 1.2, cut_height: 0.075 };
				var fire_torch = line.split(" ");
				for (var x = 0; x < fire_torch.length; x++)
				{
					if (x == 1) fire_torch_parameters.pierce_height = fire_torch[x];
					if (x == 2) fire_torch_parameters.pierce_delay = fire_torch[x];
					if (x == 3) fire_torch_parameters.cut_height = fire_torch[x];
				}
				MotionControl.send("G38.3 Z-10 F" + MotionControl.machine_parameters.machine_torch_config.z_probe_feed);
				MotionControl.send("G91 G0 Z" + this.machine_parameters.machine_torch_config.floating_head_takeup);
				MotionControl.send("G91 G0 Z" + fire_torch_parameters.pierce_height);
				MotionControl.send("M3 S1000");
				MotionControl.send("G4 P" + fire_torch_parameters.pierce_delay); //Pierce Delay
				MotionControl.send("G91 G0 Z" + (fire_torch_parameters.cut_height - fire_torch_parameters.pierce_height));
				MotionControl.send("G90"); //Back to absolute
			}
			else if (line.includes("torch_off"))
			{
				MotionControl.send("M5");
				MotionControl.send("G4 P1"); //Post Delay
				MotionControl.send("G91 G0 Z" + this.machine_parameters.machine_torch_config.clearance_height); //Retract
				MotionControl.send("G90"); //Back to absolute
			}
			else
			{
				MotionControl.send(line);
			}
		}
	}
}
MotionControl.send = function(buff)
{
	motion_control.send(buff);
}
MotionControl.send_rt = function(buff)
{
	motion_control.send_rt(buff);
}
MotionControl.tick = function()
{
	this.is_connected = motion_control.is_connected();
	var dro = motion_control.get_dro();
	if (dro.STATUS == "Idle" && this.on_idle != null)
	{
		this.on_idle();
	}
	if (dro.STATUS == "Hold" && this.on_hold != null)
	{
		this.on_hold();
	}
	//console.log(JSON.stringify(dro) + "\n");
	MotionControl.arc_ok = !dro.ARC_OK ; //Logic is inverted because arc_ok is active low
	//console.log(MotionControl.arc_ok + "\n");
	MotionControl.dro_data = { X_MCS: dro.MCS.x, Y_MCS: dro.MCS.y, X_WCS: dro.WCS.x, Y_WCS: dro.WCS.y, VELOCITY: dro.FEED, THC_ARC_VOLTAGE: FastMath.map(this.average_adc(dro.ADC), 0, 1024, 0, 10) * 50.0, THC_SET_VOLTAGE: UserInterface.thc_set_voltage.toFixed(2), STATUS: dro.STATUS };
	if (this.is_connected == true && MotionControl.dro_data.STATUS == "Run")
	{
		render.set_loop_delay(60); //Make sure motion_control has priority
	}
	else if (MotionControl.dro_data.STATUS == "Idle")
	{
		render.set_loop_delay(0);
		if (MotionControl.thc_command != "Idle")
		{
			MotionControl.thc_command = "Idle";
			motion_control.torch_cancel();
		}
	}
}
