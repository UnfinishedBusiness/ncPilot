var GcodeViewer = {};

GcodeViewer.JogCancle = {};
GcodeViewer.JogCancle.z = false;
GcodeViewer.JogCancle.axis = false;
GcodeViewer.ControlKeyDown = false;

GcodeViewer.last_file = null;
GcodeViewer.contours = []; //Stores contours for ctrl-click jump in!

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
	var contours = [];
	var contour = [];
	console.log("Parsing Gcode: " + gcode_file + "\n");
	if (gcode.parse_file(gcode_file))
	{
		for (var x = 0; x < gcode.size(); x++)
		{
			var block = gcode.get(x);
			//console.log("Block: " + JSON.stringify(block) + "\n");
			pointer = {x: block.x, y: block.y};
			if (block.g == 0)
			{
				//last_pointer = { x: pointer.x, y: pointer.y };
				//contour.push({x: block.x, y: block.y });
				contours.push(contour);
				contour = [];
			}
			if (block.g == 1)
			{
				//render.add_entity({ type: "line", start: {x: last_pointer.x + MotionControl.machine_parameters.work_offset.x, y: last_pointer.y + MotionControl.machine_parameters.work_offset.y}, end: {x: pointer.x + MotionControl.machine_parameters.work_offset.x, y: pointer.y + MotionControl.machine_parameters.work_offset.y}, color: { r: 1, g: 1, b: 1} });
				//last_pointer = { x: pointer.x, y: pointer.y };
				contour.push({x: block.x + MotionControl.machine_parameters.work_offset.x, y: block.y + MotionControl.machine_parameters.work_offset.y});
			}
		}
		contours.push(contour);
		render.add_entity({ type: "part", contours: contours, toolpaths: [], center: {x: 0, y: 0}, offset: {x: 0, y: 0, a: 0}, color: {r: 1, g: 1, b: 1} });
	}
	gcode.clear();
	console.log("Parsed Gcode in " + (time.millis() - timestamp) + "ms\n");
	this.contours = contours;
}
GcodeViewer.jump_in_to_point = function(search_point)
{
	console.log("Searching for point: " + JSON.stringify(search_point) + "\n");
	var gcode_list = [];
	var push_flag = false;
	var gcode_contents = file.get_contents(GcodeViewer.last_file);
	var lines = gcode_contents.split("\n");
	for (var x = 0; x < lines.length; x++)
	{
		if (lines[x].includes("G0"))
		{
			var line_parse = lines[x].split(" ");
			//console.log(JSON.stringify(line_parse) + "\n");
			var point = {x: 0, y: 0};
			for (var i = 0; i < line_parse.length; i++)
			{
				var value = line_parse[i];
				if (value.indexOf("X") !== -1)
				{
					//x = parseFloat(value.substr(1));
					//console.log("\X: " + value.substr(1) + "\n");
					point.x =  parseFloat(value.substr(1));
				}
				if (value.indexOf("Y") !== -1)
				{
					//y = parseFloat(value.substr(1));
					//console.log("\Y: " + value.substr(1) + "\n");
					point.y =  parseFloat(value.substr(1));
				}
			}
			if (geometry.points_match(point, search_point))
			{
				//console.log("Found Jumpin line at " + x + "\n");
				push_flag = true;
			}
		}
		if (push_flag == true)
		{
			gcode_list.push(lines[x]);
		}
	}
	MotionControl.send_gcode_from_list(gcode_list);
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
		if (key.keycode == 341 && this.ControlKeyDown == false)
		{
			this.ControlKeyDown = true;
		}
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
		if (key.keycode == 39 && GcodeViewer.JogCancle.z == false) //,
		{
			motion_control.torch_plus();
			GcodeViewer.JogCancle.z = true;
			//MotionControl.thc_command = "MUp";
			//console.log("Up!\n");
		}
		if (key.keycode == 47 && GcodeViewer.JogCancle.z == false) //.
		{
			motion_control.torch_minus();
			GcodeViewer.JogCancle.z = true;
			//MotionControl.thc_command = "MDown";
			//console.log("Down!\n");
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
		this.ControlKeyDown = false;
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
			if (this.ControlKeyDown == true)
			{
				if (ClickOnce == false)
				{
					//console.log("Ctrl-Click!\n");
					ClickOnce = true;
					var mouse_p = render.get_mouse();
					var clicked_inside_contours = [];
					for (var i = 0; i < this.contours.length; i++)
					{
						if (geometry.point_is_inside_polygon(this.contours[i], mouse_p))
						{
							clicked_inside_contours.push(i);
						}
					}
					//If we were a click inside an inside cut, we would have returned two contours.
					//Determine which one does not have any other contours inside it
					if (clicked_inside_contours.length == 2)
					{
						if (geometry.polygon_is_inside_polygon(this.contours[clicked_inside_contours[0]], this.contours[clicked_inside_contours[1]]))
						{
							console.log("Jumping into inside cut at contour: " + clicked_inside_contours[0] + "\n");
							var jump_in_contours = [];
							jump_in_contours.push(this.contours[clicked_inside_contours[0]]);
							render.add_entity({ type: "part", contours: jump_in_contours, toolpaths: [], center: {x: 0, y: 0}, offset: {x: 0, y: 0.0, a: 0}, color: {r: 0, g: 1, b: 0} });
							this.jump_in_to_point(this.contours[clicked_inside_contours[0]][0]);
						}
						else
						{
							console.log("Jumping into inside cut at contour: " + clicked_inside_contours[1] + "\n");
							var jump_in_contours = [];
							jump_in_contours.push(this.contours[clicked_inside_contours[1]]);
							render.add_entity({ type: "part", contours: jump_in_contours, toolpaths: [], center: {x: 0, y: 0}, offset: {x: 0, y: 0, a: 0}, color: {r: 0, g: 1, b: 0} });
							this.jump_in_to_point(this.contours[clicked_inside_contours[1]][0]);
						}
					}
					else if (clicked_inside_contours.length == 1)
					{
						console.log("Jumping into outside cut at contour: " + clicked_inside_contours[0] + "\n");
						var jump_in_contours = [];
						jump_in_contours.push(this.contours[clicked_inside_contours[0]]);
						render.add_entity({ type: "part", contours: jump_in_contours, toolpaths: [], center: {x: 0, y: 0}, offset: {x: 0, y: 0, a: 0}, color: {r: 0, g: 1, b: 0} });
						this.jump_in_to_point(this.contours[clicked_inside_contours[0]][0]);
					}
				}
			}
			else
			{
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
		}
		else
		{
			ClickOnce = false;
		}
	}
}