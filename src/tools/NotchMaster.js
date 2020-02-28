var NotchMaster = {};
NotchMaster.dialog = {};
NotchMaster.parameters = {};
NotchMaster.parameters.tube_length = 10;
NotchMaster.parameters.tube_diameter = 1.5;
NotchMaster.parameters.wall_thickness = 0.125;
NotchMaster.parameters.relative_angle = 0;
NotchMaster.parameters.feed_ipm = 10;
NotchMaster.parameters.pierce_height = 0.165;
NotchMaster.parameters.pierce_delay = 1.2;
NotchMaster.parameters.cut_height = 0.1;
NotchMaster.parameters.side_one = {};
NotchMaster.parameters.side_one.operation_type = 0;
NotchMaster.parameters.side_one.header_diameter = 1.750;
NotchMaster.parameters.side_one.mate_angle = 90;
NotchMaster.parameters.side_two = {};
NotchMaster.parameters.side_two.operation_type = 2;
NotchMaster.parameters.side_two.header_diameter = 1.750;
NotchMaster.parameters.side_two.mate_angle = 90;

NotchMaster.GcodeLines = [];

NotchMaster.init = function()
{
	this.dialog.id = gui.new_window("NotchMaster");
	this.dialog.tube_length = gui.add_input_double(this.dialog.id, "Tube Length", NotchMaster.parameters.tube_length);
	this.dialog.tube_diameter = gui.add_input_double(this.dialog.id, "Tube Diameter", NotchMaster.parameters.tube_diameter);
	this.dialog.wall_thickness = gui.add_input_double(this.dialog.id, "Wall Thickness", NotchMaster.parameters.wall_thickness);
	this.dialog.relative_angle = gui.add_input_double(this.dialog.id, "Relative Angle", NotchMaster.parameters.relative_angle);
	this.dialog.feed_ipm = gui.add_input_double(this.dialog.id, "Feed (IPM)", NotchMaster.parameters.feed_ipm);
	this.dialog.pierce_height = gui.add_input_double(this.dialog.id, "Pierce Height", NotchMaster.parameters.pierce_height);
	this.dialog.pierce_delay = gui.add_input_double(this.dialog.id, "Pierce Delay", NotchMaster.parameters.pierce_delay);
	this.dialog.cut_height = gui.add_input_double(this.dialog.id, "Cut Height", NotchMaster.parameters.cut_height);
	gui.separator(this.dialog.id);
	this.dialog.side_one_operation = gui.add_radiogroup(this.dialog.id, ['Notch Side 1', 'Slice Side 1'], NotchMaster.parameters.side_one.operation_type);
	this.dialog.side_one_header_diameter = gui.add_input_double(this.dialog.id, "Side 1 - Header Diameter", NotchMaster.parameters.side_one.header_diameter);
	this.dialog.side_one_mate_angle = gui.add_input_double(this.dialog.id, "Side 1 - Mate Angle", NotchMaster.parameters.side_one.mate_angle);
	gui.separator(this.dialog.id);
	this.dialog.side_two_operation = gui.add_radiogroup(this.dialog.id, ['Notch Side 2', 'Slice Side 2', 'Disable'], NotchMaster.parameters.side_two.operation_type);
	this.dialog.side_two_header_diameter = gui.add_input_double(this.dialog.id, "Side 2 - Header Diameter", NotchMaster.parameters.side_two.header_diameter);
	this.dialog.side_two_mate_angle = gui.add_input_double(this.dialog.id, "Side 2 - Mate Angle", NotchMaster.parameters.side_two.mate_angle);
	gui.separator(this.dialog.id);
	this.dialog.run = gui.add_button(this.dialog.id, "Run");
	gui.sameline(this.dialog.id);
	this.dialog.view = gui.add_button(this.dialog.id, "View");
	gui.sameline(this.dialog.id);
	this.dialog.close_button = gui.add_button(this.dialog.id, "Close");
	gui.sameline(this.dialog.id);
	this.dialog.save_button = gui.add_button(this.dialog.id, "Save");
	gui.sameline(this.dialog.id);
	this.dialog.open_button = gui.add_button(this.dialog.id, "Open");
	gui.show(this.dialog.id, false); //Default to hidden
}
NotchMaster.tick = function()
{
	if (gui.get_button(this.dialog.id, this.dialog.save_button))
	{
		this.parameters.tube_length = gui.get_input_double(this.dialog.id, this.dialog.tube_length);
		this.parameters.tube_diameter = gui.get_input_double(this.dialog.id, this.dialog.tube_diameter);
		this.parameters.wall_thickness = gui.get_input_double(this.dialog.id, this.dialog.wall_thickness);
		this.parameters.relative_angle = gui.get_input_double(this.dialog.id, this.dialog.relative_angle);
		this.parameters.feed_ipm = gui.get_input_double(this.dialog.id, this.dialog.feed_ipm);
		this.parameters.pierce_height = gui.get_input_double(this.dialog.id, this.dialog.pierce_height);
		this.parameters.pierce_delay = gui.get_input_double(this.dialog.id, this.dialog.pierce_delay);
		this.parameters.cut_height = gui.get_input_double(this.dialog.id, this.dialog.cut_height);
		this.parameters.side_one.operation_type = gui.get_radiogroup(this.dialog.id, this.dialog.side_one_operation);
		this.parameters.side_one.header_diameter = gui.get_input_double(this.dialog.id, this.dialog.side_one_header_diameter);
		this.parameters.side_one.mate_angle = gui.get_input_double(this.dialog.id, this.dialog.side_one_mate_angle);
		this.parameters.side_two.operation_type = gui.get_radiogroup(this.dialog.id, this.dialog.side_two_operation);
		this.parameters.side_two.header_diameter = gui.get_input_double(this.dialog.id, this.dialog.side_two_header_diameter);
		this.parameters.side_two.mate_angle = gui.get_input_double(this.dialog.id, this.dialog.side_two_mate_angle);
		file.put_contents(file_dialog.save({ filter: ["*.notch"]}), JSON.stringify(this.parameters));
	}
	if (gui.get_button(this.dialog.id, this.dialog.open_button))
	{
		this.parameters = JSON.parse(file.get_contents(file_dialog.open({ filter: ["*.notch"]})));
		this.hide();
		this.init();
		this.show();
	}

	if (gui.get_button(this.dialog.id, this.dialog.close_button))
	{
		this.hide();
	}
	if (gui.get_button(this.dialog.id, this.dialog.view))
	{
		this.ProcessNotches();
	}
	if (gui.get_button(this.dialog.id, this.dialog.run))
	{
		/*for (var x = 0; x < this.GcodeLines.length; x++)
		{
			console.log(this.GcodeLines[x] + "\n");
		}*/
		MotionControl.send_gcode_from_list(this.GcodeLines);
		//motion_control.cycle_start();
	}
}
NotchMaster.show = function()
{
	gui.show(this.dialog.id, true);
}
NotchMaster.hide = function()
{
	gui.show(this.dialog.id, false);
}
NotchMaster.degree_sine = function(degree)
{
	return Math.sin(this.deg2rad(degree));
}
NotchMaster.degree_cosine = function(degree)
{
	return Math.cos(this.deg2rad(degree));
}
NotchMaster.degree_tangent = function(degree)
{
	return Math.tan(this.deg2rad(degree));
}
NotchMaster.deg2rad = function(degrees)
{
	return degrees * (3.1415926535897932384 / 180);
}
NotchMaster.CalculateNotchExtent = function(points)
{
	var biggestY = -1000000;
	for (var x = 0; x < points.length; x++)
	{
		if (points[x].y > biggestY) biggestY = points[x].y;
	}
	var smallestY = 1000000;
	for (var x = 0; x < points.length; x++)
	{
		if (points[x].y < smallestY) smallestY = points[x].y;
	}
	return (biggestY - smallestY);
}
NotchMaster.CalculateNotch = function(tube_diameter, header_diameter, wall_thickness, angle, invert)
{
	//console.log("tube_diameter: " + tube_diameter + "\n");
	//console.log("header_diameter: " + header_diameter + "\n");
	//console.log("wall_thickness: " + wall_thickness + "\n");
	//console.log("angle: " + angle + "\n");
	var RO = tube_diameter / 2.0;
	var RU = header_diameter / 2.0
	var RI = RO - wall_thickness;
	var AF = angle;
	var ID = 0;
	var A_Axis_Angle = -1;
	var points = [];
	for (var i = 0; i < 361; i++)
	{
		var Y;
		if (invert == true)
		{
			Y = (Math.sqrt(RU **2 - ((RI * this.degree_sine(ID)) **2)) / this.degree_sine(AF) - this.degree_tangent(90 - AF) * RI * this.degree_cosine(ID)) * -1;
		}
		else
		{
			Y = (Math.sqrt(RU **2 - ((RI * this.degree_sine(ID)) **2)) / this.degree_sine(AF) - this.degree_tangent(90 - AF) * RI * this.degree_cosine(ID));
		}
		ID += 1;
		A_Axis_Angle += 1;
		points.push({x: A_Axis_Angle * 1000, y: Y.toFixed(4) * 1000});
	}
	//console.log("Points before simplification: " + points.length + "\n");
	var simplified_points = simplify(points, 5, true);
	//console.log("Points after simplification: " + simplified_points.length + "\n");
	//Output points for debugging
	points = [];
	for (var x = 0; x < simplified_points.length; x++)
	{
		points.push({ x: FastMath.map(simplified_points[x].x / 1000, 0, 360, 0, tube_diameter * 3.14), y: simplified_points[x].y / 1000});
	}
	//Right now points array includes our simplified list, now we need to offset it
	if (invert == true)
	{
		var biggestY = -1000000;
		for (var x = 0; x < points.length; x++)
		{
			if (points[x].y > biggestY) biggestY = points[x].y;
		}
		//console.log("Biggest Y = " + biggestY + "\n");
		for (var x = 0; x < points.length; x++)
		{
			points[x].y -= biggestY;
			//console.log("X: " + points[x].x + " Y: " + points[x].y + "\n");
		}
	}
	else
	{
		var smallestY = 1000000;
		for (var x = 0; x < points.length; x++)
		{
			if (points[x].y < smallestY) smallestY = points[x].y;
		}
		for (var x = 0; x < points.length; x++)
		{
			points[x].y -= smallestY;
			//console.log("X: " + points[x].x + " Y: " + points[x].y + "\n");
		}
	}
	return points;
}
NotchMaster.ProcessNotches = function()
{
	//Visualize in gcode visual
	this.GcodeLines = [];
	GcodeViewer.clear();
	//Draw a flat visual of our base tube
	var tube_diameter = gui.get_input_double(this.dialog.id, this.dialog.tube_diameter);
	var tube_length = gui.get_input_double(this.dialog.id, this.dialog.tube_length);
	var relative_angle = gui.get_input_double(this.dialog.id, this.dialog.relative_angle);
	var feed_deg_min = (gui.get_input_double(this.dialog.id, this.dialog.feed_ipm) * 360) / (tube_diameter * 3.14);
	var notch_one_extent = 0;
	var notch_two_extent = 0;

	var fire_torch = "fire_torch " + gui.get_input_double(this.dialog.id, this.dialog.pierce_height) + " " + gui.get_input_double(this.dialog.id, this.dialog.pierce_delay) + " " + gui.get_input_double(this.dialog.id, this.dialog.cut_height);

	var side_one_operation = gui.get_radiogroup(this.dialog.id, this.dialog.side_one_operation);
	if (side_one_operation == 0) //Notch Side 1
	{
		//Draw side one notch
		var notch_one = this.CalculateNotch(gui.get_input_double(this.dialog.id, this.dialog.tube_diameter), gui.get_input_double(this.dialog.id, this.dialog.side_one_header_diameter), gui.get_input_double(this.dialog.id, this.dialog.wall_thickness), gui.get_input_double(this.dialog.id, this.dialog.side_one_mate_angle), true);
		notch_one_extent = this.CalculateNotchExtent(notch_one);
		var last_pointer = { x: notch_one[0].x, y: notch_one[0].y };
		this.GcodeLines.push("G0 X" + Math.round(FastMath.map(last_pointer.x.toFixed(4), 0, tube_diameter * 3.14, 0, 360))+ " Y" + last_pointer.y.toFixed(4));
		this.GcodeLines.push(fire_torch);
		for (var x = 1; x < notch_one.length; x++)
		{
			render.add_entity({ type: "line", start: {x: last_pointer.x + MotionControl.machine_parameters.work_offset.x, y: last_pointer.y + MotionControl.machine_parameters.work_offset.y}, end: {x: notch_one[x].x + MotionControl.machine_parameters.work_offset.x, y: notch_one[x].y + MotionControl.machine_parameters.work_offset.y}, color: { r: 1, g: 1, b: 1} });
			this.GcodeLines.push("G1 X" + Math.round(FastMath.map(last_pointer.x.toFixed(4), 0, tube_diameter * 3.14, 0, 360))+ " Y" + last_pointer.y.toFixed(4) + " F" + feed_deg_min.toFixed(4));
			last_pointer = { x: notch_one[x].x, y: notch_one[x].y };
		}
		this.GcodeLines.push("G1 X" + Math.round(FastMath.map(last_pointer.x.toFixed(4), 0, tube_diameter * 3.14, 0, 360)) + " Y" + last_pointer.y.toFixed(4) + " F" + feed_deg_min.toFixed(4));
		this.GcodeLines.push("torch_off");
	}
	else if (side_one_operation == 1) //Slice Side 1
	{
		//Draw side one slice
		this.GcodeLines.push("G0 X0 Y0");
		this.GcodeLines.push(fire_torch);
		this.GcodeLines.push("G1 X360 Y0 F" + feed_deg_min.toFixed(4));
		this.GcodeLines.push("torch_off");
		render.add_entity({ type: "line", start: {x: 0 + MotionControl.machine_parameters.work_offset.x, y: 0 + MotionControl.machine_parameters.work_offset.y}, end: {x: (tube_diameter * 3.14) + MotionControl.machine_parameters.work_offset.x, y: 0 + MotionControl.machine_parameters.work_offset.y}, color: { r: 1, g: 1, b: 1} });
	}
	
	
	var side_two_operation = gui.get_radiogroup(this.dialog.id, this.dialog.side_two_operation);
	if (side_two_operation == 0) //Notch Side 2
	{
		//Draw side two notch
		var notch_two = this.CalculateNotch(gui.get_input_double(this.dialog.id, this.dialog.tube_diameter), gui.get_input_double(this.dialog.id, this.dialog.side_two_header_diameter), gui.get_input_double(this.dialog.id, this.dialog.wall_thickness), gui.get_input_double(this.dialog.id, this.dialog.side_two_mate_angle), false);
		notch_two_extent = this.CalculateNotchExtent(notch_two);
		tube_length += notch_one_extent;
		tube_length += notch_two_extent;
		for (var x = 0; x < notch_two.length; x++)
		{
			notch_two[x].y -= tube_length;
		}
		var last_pointer = { x: notch_two[0].x, y: notch_two[0].y };
		this.GcodeLines.push("G0 X" + Math.round(FastMath.map(last_pointer.x.toFixed(4), 0, tube_diameter * 3.14, 0, 360)) + " Y" + last_pointer.y.toFixed(4));
		this.GcodeLines.push(fire_torch);
		for (var x = 1; x < notch_two.length; x++)
		{
			render.add_entity({ type: "line", start: {x: last_pointer.x + MotionControl.machine_parameters.work_offset.x, y: last_pointer.y + MotionControl.machine_parameters.work_offset.y}, end: {x: notch_two[x].x + MotionControl.machine_parameters.work_offset.x, y: notch_two[x].y + MotionControl.machine_parameters.work_offset.y}, color: { r: 1, g: 1, b: 1} });
			this.GcodeLines.push("G1 X" + Math.round(FastMath.map(last_pointer.x.toFixed(4), 0, tube_diameter * 3.14, 0, 360)) + " Y" + last_pointer.y.toFixed(4) + " F" + feed_deg_min.toFixed(4));
			last_pointer = { x: notch_two[x].x, y: notch_two[x].y };
		}
		this.GcodeLines.push("G1 X" + Math.round(FastMath.map(last_pointer.x.toFixed(4), 0, tube_diameter * 3.14, 0, 360)) + " Y" + last_pointer.y.toFixed(4) + " F" + feed_deg_min.toFixed(4));
		this.GcodeLines.push("torch_off");
	}
	else if (side_two_operation == 1) //Slice Side 2
	{
		this.GcodeLines.push("G0 X0 Y" + (tube_length * -1));
		this.GcodeLines.push(fire_torch);
		this.GcodeLines.push("G1 X360 Y" + (tube_length * -1) + " F" + feed_deg_min.toFixed(4));
		this.GcodeLines.push("torch_off");
		//Draw side two slice
		render.add_entity({ type: "line", start: {x: 0 + MotionControl.machine_parameters.work_offset.x, y: (tube_length * -1) + MotionControl.machine_parameters.work_offset.y}, end: {x: (tube_diameter * 3.14) + MotionControl.machine_parameters.work_offset.x, y: (tube_length * -1) + MotionControl.machine_parameters.work_offset.y}, color: { r: 1, g: 1, b: 1} });
	}
	else if (side_two_operation == 2) //Disable Side 2
	{

	}

	if ((side_one_operation == 0 || side_one_operation == 1) && (side_two_operation == 0 || side_two_operation == 1))
	{
		//Draw Green Sides
		//render.add_entity({ type: "line", start: {x: 0 + MotionControl.machine_parameters.work_offset.x, y: 0 + MotionControl.machine_parameters.work_offset.y}, end: {x: (tube_diameter * 3.14) + MotionControl.machine_parameters.work_offset.x, y: 0 + MotionControl.machine_parameters.work_offset.y}, color: { r: 0, g: 1, b: 0} });
		//render.add_entity({ type: "line", start: {x: (tube_diameter * 3.14) + MotionControl.machine_parameters.work_offset.x, y: 0 + MotionControl.machine_parameters.work_offset.y}, end: {x: (tube_diameter * 3.14) + MotionControl.machine_parameters.work_offset.x, y: (tube_length * -1) + MotionControl.machine_parameters.work_offset.y}, color: { r: 0, g: 1, b: 0} });
		//render.add_entity({ type: "line", start: {x: (tube_diameter * 3.14) + MotionControl.machine_parameters.work_offset.x, y: (tube_length * -1) + MotionControl.machine_parameters.work_offset.y}, end: {x: 0 + MotionControl.machine_parameters.work_offset.x, y: (tube_length * -1) + MotionControl.machine_parameters.work_offset.y}, color: { r: 0, g: 1, b: 0} });
		//render.add_entity({ type: "line", start: {x: 0 + MotionControl.machine_parameters.work_offset.x, y: (tube_length * -1) + MotionControl.machine_parameters.work_offset.y}, end: {x: 0 + MotionControl.machine_parameters.work_offset.x, y: 0 + MotionControl.machine_parameters.work_offset.y}, color: { r: 0, g: 1, b: 0} });

		//Draw Relative Angle
		render.add_entity({ type: "line", start: {x: 0 + MotionControl.machine_parameters.work_offset.x, y: (((tube_length * -1) / 2.0) - 0.250) + MotionControl.machine_parameters.work_offset.y}, end: {x: (tube_diameter * 3.14) + MotionControl.machine_parameters.work_offset.x, y: (((tube_length * -1) / 2.0) - 0.250) + MotionControl.machine_parameters.work_offset.y}, color: { r: 0, g: 0, b: 0} });
		render.add_entity({ type: "text", position: {x: ((tube_diameter * 3.14) / 2.0) + MotionControl.machine_parameters.work_offset.x - 1.0, y: ((tube_length * -1) / 2.0) + MotionControl.machine_parameters.work_offset.y }, text: "Relative Angle: " + relative_angle + " Deg", height: 0.125 });
		render.add_entity({ type: "line", start: {x: 0 + MotionControl.machine_parameters.work_offset.x, y: (((tube_length * -1) / 2.0) + 0.250) + MotionControl.machine_parameters.work_offset.y}, end: {x: (tube_diameter * 3.14) + MotionControl.machine_parameters.work_offset.x, y: (((tube_length * -1) / 2.0) + 0.250) + MotionControl.machine_parameters.work_offset.y}, color: { r: 0, g: 0, b: 0} });
		
		//Draw Dimentions
		if (side_one_operation == 0 && side_two_operation == 0)
		{
			//Start points
			var line = { start: {x: notch_one[0].x + MotionControl.machine_parameters.work_offset.x, y: notch_one[0].y + MotionControl.machine_parameters.work_offset.y}, end: {x: notch_two[0].x + MotionControl.machine_parameters.work_offset.x, y: notch_two[0].y + MotionControl.machine_parameters.work_offset.y} };
			render.add_entity({ type: "line", start: line.start, end: line.end, color: { r: 0, g: 0, b: 0} });
			var dist = this.get_distance(line.start, line.end);
			var mid = this.get_midpoint(line.start, line.end);
			render.add_entity({ type: "text", position: {x: mid.x + 0.1, y: mid.y + 1.5}, text: dist.toFixed(4) + "\"", height: 0.125 });

			//Middle Points
			line = { start: {x: notch_one[Math.round(notch_one.length / 2) -1].x + MotionControl.machine_parameters.work_offset.x, y: notch_one[Math.round(notch_one.length / 2) -1].y + MotionControl.machine_parameters.work_offset.y}, end: {x: notch_two[Math.round(notch_two.length / 2) -1].x + MotionControl.machine_parameters.work_offset.x, y: notch_two[Math.round(notch_two.length / 2) -1].y + MotionControl.machine_parameters.work_offset.y} };
			console.log(JSON.stringify(line) + "\n");
			render.add_entity({ type: "line", start: line.start, end: line.end, color: { r: 0, g: 0, b: 0} });
			var dist = this.get_distance(line.start, line.end);
			var mid = this.get_midpoint(line.start, line.end);
			render.add_entity({ type: "text", position: {x: mid.x + 0.1, y: mid.y + 1.5}, text: dist.toFixed(4) + "\"", height: 0.125 });
		}
	}
}
NotchMaster.get_distance = function(point_one, point_two)
{
	var diff = {};
	diff.x = point_one.x - point_two.x;
	diff.y = point_one.y - point_two.y;
	return Math.sqrt((diff.x * diff.x) + (diff.y * diff.y));
}
NotchMaster.get_midpoint = function(point_one, point_two)
{
	var p = {};
	p.x = (point_one.x + point_two.x) / 2;
	p.y = (point_one.y + point_two.y) / 2;
	return p;
}