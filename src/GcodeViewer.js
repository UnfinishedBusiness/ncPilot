var GcodeViewer = {};

GcodeViewer.last_file = null;

GcodeViewer.clear = function()
{
	render.clear();
	render.add_entity({ type: "rectangle", size: {x: MotionControl.machine_parameters.machine_extents.x, y: MotionControl.machine_parameters.machine_extents.x}, color: {r: 0.3, g: 0.01, b: 0.01} });
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
				render.add_entity({ type: "line", start: {x: last_pointer.x + MotionControl.machine_parameters.work_offset.x, y: last_pointer.y + MotionControl.machine_parameters.work_offset.y}, end: {x: pointer.x + MotionControl.machine_parameters.work_offset.x, y: pointer.y + + MotionControl.machine_parameters.work_offset.y}, color: { r: 1, g: 1, b: 1} });
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
}
GcodeViewer.tick = function()
{
	render.show_crosshair({ pos: {x: MotionControl.dro_data.X_MCS, y: MotionControl.dro_data.Y_MCS} });
	var scroll = render.get_scroll();
	if (scroll.horizontal != 0 || scroll.vertical != 0)
	{
		//console.log(JSON.stringify(scroll) + "\n");
		render.pan((scroll.horizontal * 0.05) / render.get_zoom(), (scroll.vertical * -0.05) / render.get_zoom());
	}
	var key = gui.get_keyboard();
	if (key.keycode > 0)
	{
		//console.log(JSON.stringify(key) + "\n");
		if (key.char == "=")
		{
			render.zoom(0.05 * render.get_zoom());
		}
		if (key.char == "-")
		{
			render.zoom(-(0.05 * render.get_zoom()));
		}
		if (key.keycode == 265) //Up
		{
			render.pan(0, ((0.01 / render.get_zoom())));
		}
		if (key.keycode == 264) //Down
		{
			render.pan(0, (-(0.01 / render.get_zoom())));
		}
		if (key.keycode == 263) //Left
		{
			render.pan((-(0.01 / render.get_zoom())), 0);
		}
		if (key.keycode == 262) //Right
		{
			render.pan(((0.01 / render.get_zoom())), 0);
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
					WayPoint.x -= MotionControl.machine_parameters.work_offset.x;
					WayPoint.y -= MotionControl.machine_parameters.work_offset.y;
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