var MotionControl = {};
MotionControl.GStack = [];
MotionControl.machine_parameters = {
	machine_extents: { x: 45.5, y: 45.5 },
	machine_axis_invert: { x: 0, y1: 1, y2: 0, z: 0 },
	machine_axis_scale: { x: 1034.5, y: 1034.5, z: 1000 },
	machine_torch_config: { z_rapid_feed: 25, z_probe_feed: 20, floating_head_takeup: 0.1, clearance_height: 2 },
	machine_thc_config: { pin: "A19", comp_vel: 5, filter: 5000, adc_at_zero: 35, adc_at_one_hundred: 650 },
	work_offset: {x: 0, y: 0},
  };
MotionControl.way_point = null;
MotionControl.is_connected = false;
MotionControl.dro_data = { X_MCS: 0.0000, Y_MCS: 0.0000, X_WCS: 0.0000, Y_WCS: 0.0000, VELOCITY: 0.0, THC_ARC_VOLTAGE: 0.0, THC_SET_VOLTAGE: 0.0, STATUS: "Halt" };
MotionControl.on_idle = null;

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
	var contents = "";
	if (file.open("machine_parameters.json", "r"))
	{
		while(file.lines_available())
		{
			contents += file.read();
		}
		//console.log("" + contents + "\n");
		this.machine_parameters = JSON.parse(contents);
		motion_control.set_work_offset({x: this.machine_parameters.work_offset.x, y:  this.machine_parameters.work_offset.y});
		file.close();
	}
}
MotionControl.ProgramAbort = function()
{
	motion_control.clear_stack();
	this.on_idle = function(){
		console.log("Sending soft reset!\n");
	};
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
			console.log("Going to waypoint!\n");
			this.send("G53 G0 X" + this.way_point.x.toFixed(4) + " Y" + this.way_point.y.toFixed(4));
			//this.send("G54");
		}
	}
}
MotionControl.init = function()
{
	motion_control.set_port("Arduino");
	motion_control.set_dro_interval(75);
	this.PullParameters();
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
	//console.log(JSON.stringify(dro) + "\n");
	MotionControl.dro_data = { X_MCS: dro.MCS.x, Y_MCS: dro.MCS.y, X_WCS: dro.WCS.x, Y_WCS: dro.WCS.y, VELOCITY: dro.FEED, THC_ARC_VOLTAGE: dro.ADC, THC_SET_VOLTAGE: 0.0, STATUS: dro.STATUS };
}
