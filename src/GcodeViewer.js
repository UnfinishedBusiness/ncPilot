var GcodeViewer = {};

GcodeViewer.JogCancle = {};
GcodeViewer.JogCancle.z = false;
GcodeViewer.JogCancle.axis = false;

GcodeViewer.last_file = null;

GcodeViewer.clear = function()
{
	render.clear();
	render.add_entity({ type: "rectangle", size: {x: MotionControl.machine_parameters.machine_extents.x, y: MotionControl.machine_parameters.machine_extents.y}, color: {r: 0.3, g: 0.01, b: 0.01} });
	render.add_entity({ type: "line", start: {x: 0, y: 0}, end: {x: MotionControl.machine_parameters.machine_extents.x, y: 0}, color: { r: 0, g: 0, b: 1} });
	render.add_entity({ type: "line", start: {x: MotionControl.machine_parameters.machine_extents.x, y: 0}, end: {x: MotionControl.machine_parameters.machine_extents.x, y: MotionControl.machine_parameters.machine_extents.y}, color: { r: 0, g: 0, b: 1} });
	render.add_entity({ type: "line", start: {x: MotionControl.machine_parameters.machine_extents.x, y: MotionControl.machine_parameters.machine_extents.y}, end: {x: 0, y: MotionControl.machine_parameters.machine_extents.y}, color: { r: 0, g: 0, b: 1} });
	render.add_entity({ type: "line", start: {x: 0, y: MotionControl.machine_parameters.machine_extents.y}, end: {x: 0, y: 0}, color: { r: 0, g: 0, b: 1} });
}
GcodeViewer.parse_gcode = function (gcode_file)
{	
	this.last_file = gcode_file;
	GcodeViewer.clear();
	var timestamp = time.millis();
	var last_pointer = { x: 0, y: 0 };
	var pointer = { x: 0, y: 0 };
	console.log("Parsing Gcode: " + gcode_file + "\n");
	if (gcode.parse_file(gcode_file))
	{
		for (var x = 0; x < gcode.size(); x++)
		{
			var block = gcode.get(x);
			pointer = {x: block.x, y: block.y};
			if (block.g == 0)
			{
				last_pointer = { x: pointer.x, y: pointer.y };
			}
			if (block.g == 1)
			{
				render.add_entity({ type: "line", start: {x: last_pointer.x + MotionControl.machine_parameters.work_offset.x, y: last_pointer.y + MotionControl.machine_parameters.work_offset.y}, end: {x: pointer.x + MotionControl.machine_parameters.work_offset.x, y: pointer.y + MotionControl.machine_parameters.work_offset.y}, color: { r: 1, g: 1, b: 1} });
				last_pointer = { x: pointer.x, y: pointer.y };
			}
		}
	}
	gcode.clear();
	console.log("Parsed Gcode in " + (time.millis() - timestamp) + "ms\n");
}
GcodeViewer.init = function()
{
	GcodeViewer.OnePress = false;
	GcodeViewer.ClickOnce = false; //Used to keep track of one click at a time
	render.set_style({ background_color: { r: 0, g: 0, b: 0.2, a: 1 }});
	render.show_crosshair({ visable: true });
	GcodeViewer.clear();
	render.zoom(40);
}
GcodeViewer.tick = function()
{
	render.show_crosshair({ pos: {x: MotionControl.dro_data.X_MCS, y: MotionControl.dro_data.Y_MCS} });
	var scroll = render.get_scroll();
	if (scroll.horizontal != 0 || scroll.vertical != 0)
	{
		var old_zoom = render.get_zoom();
		if (scroll.vertical > 0)
		{
			render.zoom(0.125 * render.get_zoom());
		}
		else
		{
			render.zoom(-0.125 * render.get_zoom());
		}
		var scalechange = old_zoom - render.get_zoom();
		var pan_x = render.get_mouse().x * scalechange;
		var pan_y = render.get_mouse().y * scalechange;
		render.pan(pan_x, pan_y);
	}
	var key = gui.get_keyboard();
	if (key.keycode > 0)
	{
		//console.log(JSON.stringify(key) + "\n");
		if (key.keycode == 265 && GcodeViewer.JogCancle.axis == false && MotionControl.dro_data.STATUS == "Idle") //Up Arrow
		{
			GcodeViewer.JogCancle.axis = true;
			MotionControl.send("G53 G1 Y" + MotionControl.machine_parameters.machine_extents.y + " F" + gui.get_slider(UserInterface.control_window.window, UserInterface.control_window.jog_speed));
		}
		if (key.keycode == 264 && GcodeViewer.JogCancle.axis == false && MotionControl.dro_data.STATUS == "Idle") //Down Arrow
		{
			GcodeViewer.JogCancle.axis = true;
			MotionControl.send("G53 G1 Y0" + " F" + gui.get_slider(UserInterface.control_window.window, UserInterface.control_window.jog_speed));
		}
		if (key.keycode == 263 && GcodeViewer.JogCancle.axis == false && MotionControl.dro_data.STATUS == "Idle") //Left Arrow
		{
			GcodeViewer.JogCancle.axis = true;
			MotionControl.send("G53 G1 X0" + " F" + gui.get_slider(UserInterface.control_window.window, UserInterface.control_window.jog_speed));
		}
		if (key.keycode == 262 && GcodeViewer.JogCancle.axis == false && MotionControl.dro_data.STATUS == "Idle") //Right Arrow
		{
			GcodeViewer.JogCancle.axis = true;
			MotionControl.send("G53 G1 X" + MotionControl.machine_parameters.machine_extents.x + " F" + gui.get_slider(UserInterface.control_window.window, UserInterface.control_window.jog_speed));
		}
		if (key.keycode == 266 && GcodeViewer.JogCancle.axis == false && MotionControl.dro_data.STATUS == "Idle") //Page Up when machine is not running
		{
			GcodeViewer.JogCancle.axis = true;
			MotionControl.send("G53 G1 Z0" + " F" + gui.get_slider(UserInterface.control_window.window, UserInterface.control_window.jog_speed));
			//console.log("Jog Z+\n");
		}
		if (key.keycode == 267 && GcodeViewer.JogCancle.axis == false && MotionControl.dro_data.STATUS == "Idle") //Page Down when machine is not running
		{
			GcodeViewer.JogCancle.axis = true;
			MotionControl.send("G53 G1 Z-5" + " F" + gui.get_slider(UserInterface.control_window.window, UserInterface.control_window.jog_speed));
			//console.log("Jog Z-\n");
		}
		if (key.keycode == 44 && GcodeViewer.JogCancle.z == false) //, when machine is running
		{
			motion_control.torch_plus();
			GcodeViewer.JogCancle.z = true;
			MotionControl.thc_command = "Up";
		}
		if (key.keycode == 46 && GcodeViewer.JogCancle.z == false) //. Down when machine is running
		{
			motion_control.torch_minus();
			GcodeViewer.JogCancle.z = true;
			MotionControl.thc_command = "Down";
		}
		if (key.keycode == 32 && GcodeViewer.OnePress == false) //Space
		{
			GcodeViewer.OnePress = true;
		}
		if (key.keycode == 290 && GcodeViewer.OnePress == false) //F1
		{
			GcodeViewer.OnePress = true;
			motion_control.cycle_start();
		}
		if (key.char == "=")
		{
			render.zoom(0.05 * render.get_zoom());
		}
		if (key.char == "-")
		{
			render.zoom(-(0.05 * render.get_zoom()));
		}
		if (key.keycode == 87) //Up
		{
			render.pan(0, ((30 / render.get_zoom())));
		}
		if (key.keycode == 83) //Down
		{
			render.pan(0, (-(30 / render.get_zoom())));
		}
		if (key.keycode == 65) //Left
		{
			render.pan((-(30 / render.get_zoom())), 0);
		}
		if (key.keycode == 68) //Right
		{
			render.pan(((30 / render.get_zoom())), 0);
		}
		if (key.keycode == 258 && GcodeViewer.OnePress == false) //Tab
		{
			if (MotionControl.way_point != null)
			{
				//console.log("Going to waypoint position: " + JSON.stringify(MotionControl.way_point) + "\n");
				MotionControl.go_to_waypoint();
			}
			GcodeViewer.OnePress = true;
		}
		if (key.keycode == 256 && GcodeViewer.OnePress == false) //Escape
		{
			MotionControl.ProgramAbort();
		}
	}
	else
	{
		GcodeViewer.OnePress = false;
		if (GcodeViewer.JogCancle.axis == true)
		{
			GcodeViewer.JogCancle.axis = false;
			MotionControl.ProgramAbort();
		}
		else if (GcodeViewer.JogCancle.z == true)
		{
			GcodeViewer.JogCancle.z = false;
			motion_control.torch_cancel();
			MotionControl.thc_command = "Idle";
		}
	}

	var mouse = gui.get_mouse_click();
	{
		if (mouse.keycode > -1)
		{
			//console.log("keycode: " + mouse.keycode + "\n");
			if (mouse.keycode == 0 && ClickOnce == false)
			{
				var mouse_p = render.get_mouse();
				//console.log("Clicked: " + JSON.stringify(mouse_p) + "\n");
				if (mouse_p.x < MotionControl.machine_parameters.machine_extents.x && mouse_p.x > 0 && mouse_p.y < MotionControl.machine_parameters.machine_extents.y && mouse_p.y > 0)
				{
					//console.log("Clicked inside machine boundry!\n");
					var WayPoint = mouse_p;
					MotionControl.set_waypoint(mouse_p);
				}
				ClickOnce = true;
			}
		}
		else
		{
			ClickOnce = false;
		}
	}
}