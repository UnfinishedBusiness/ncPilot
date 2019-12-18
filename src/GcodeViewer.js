var GcodeViewer = {};

GcodeViewer.JogCancle = {};
GcodeViewer.JogCancle.z = false;

GcodeViewer.last_file = null;

GcodeViewer.clear = function()
{
	render.clear();
	render.add_entity({ type: "rectangle", size: {x: MotionControl.machine_parameters.machine_extents.x, y: MotionControl.machine_parameters.machine_extents.x}, color: {r: 0.3, g: 0.01, b: 0.01} });
}
GcodeViewer.parse_gcode = function (gcode_file)
{	
	var contour_stack = [];
	var contour = [];
	
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
				contour_stack.push({feed: block.f, path: contour});
				contour = [];
			}
			if (block.g == 1)
			{
				contour.push({ type: "line", start: {x: last_pointer.x + MotionControl.machine_parameters.work_offset.x, y: last_pointer.y + MotionControl.machine_parameters.work_offset.y}, end: {x: pointer.x + MotionControl.machine_parameters.work_offset.x, y: pointer.y + MotionControl.machine_parameters.work_offset.y}, color: { r: 1, g: 1, b: 1} });
				render.add_entity({ type: "line", start: {x: last_pointer.x + MotionControl.machine_parameters.work_offset.x, y: last_pointer.y + MotionControl.machine_parameters.work_offset.y}, end: {x: pointer.x + MotionControl.machine_parameters.work_offset.x, y: pointer.y + MotionControl.machine_parameters.work_offset.y}, color: { r: 1, g: 1, b: 1} });
				last_pointer = { x: pointer.x, y: pointer.y };
			}
		}
	}
	MotionPlanner.PlannedGcodeStack = [];
	for (var x = 0; x < contour_stack.length; x++)
	{
		MotionPlanner.PlannedGcodeStack.push(MotionPlanner.plan(contour_stack[x].path, 150, 0.030, 15, 5, contour_stack[x].feed));
	}
	gcode.clear();
	console.log("Parsed & Planned Gcode in " + (time.millis() - timestamp) + "ms\n");
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
		if (key.keycode == 266 && GcodeViewer.JogCancle.z == false) //Page Up
		{
			//MotionControl.send("M5");
			//MotionControl.send("G53 G0 Z0");
			MotionControl.send_rt(">");
			GcodeViewer.JogCancle.z = true;
			//MotionControl.send("M3 S1000");
		}
		if (key.keycode == 267 && GcodeViewer.JogCancle.z == false) //Page Down
		{
			//MotionControl.send("G38.2 Z-100 F50");
			//MotionControl.send("G91 G0 Z0.200");
			//MotionControl.send("G91 G0 Z0.5");
			//MotionControl.send("G90");
			//MotionControl.send("M5");
			MotionControl.send_rt("<");
			GcodeViewer.JogCancle.z = true;
		}
		if (key.keycode == 32 && GcodeViewer.OnePress == false) //Space
		{
			//this.parse_gcode("indian.nc");
			//MotionControl.send_rt("$0=50");
			//MotionControl.send_rt("&");
			GcodeViewer.OnePress = true;
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
		if (GcodeViewer.JogCancle.z)
		{
			GcodeViewer.JogCancle.z = false;
			MotionControl.send_rt("^");
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