var NotchMaster = {};
NotchMaster.dialog = {};
NotchMaster.parameters = {};
NotchMaster.parameters.tube_length = 10;
NotchMaster.parameters.relative_angle = 0;
NotchMaster.parameters.side_one = {};
NotchMaster.parameters.side_one.operation_type = "Notch";
NotchMaster.parameters.side_one.branch_diameter = 1.750;
NotchMaster.parameters.side_one.header_diameter = 1.750;
NotchMaster.parameters.side_one.wall_thickness = 0.120;
NotchMaster.parameters.side_one.mate_angle = 90;
NotchMaster.parameters.side_two = {};
NotchMaster.parameters.side_two.operation_type = "Disabled";
NotchMaster.parameters.side_two.branch_diameter = 1.750;
NotchMaster.parameters.side_two.header_diameter = 1.750;
NotchMaster.parameters.side_two.wall_thickness = 0.120;
NotchMaster.parameters.side_two.mate_angle = 90;

NotchMaster.init = function()
{
	this.dialog.id = gui.new_window("NotchMaster");
	this.dialog.tube_length = gui.add_input_double(this.dialog.id, "Tube Length", NotchMaster.parameters.tube_length);
	this.dialog.tube_length = gui.add_input_double(this.dialog.id, "Relative Angle", NotchMaster.parameters.relative_angle);
	gui.separator(this.dialog.id);
	this.dialog.side_one_operation = gui.add_radiogroup(this.dialog.id, ['Notch Side 1', 'Slice Side 1']);
	this.dialog.side_one_branch_diameter = gui.add_input_double(this.dialog.id, "Side 1 - Branch Diameter", NotchMaster.parameters.side_one.branch_diameter);
	this.dialog.side_one_header_diameter = gui.add_input_double(this.dialog.id, "Side 1 - Header Diameter", NotchMaster.parameters.side_one.header_diameter);
	this.dialog.side_one_wall_thickness = gui.add_input_double(this.dialog.id, "Side 1 - Wall Thickness", NotchMaster.parameters.side_one.wall_thickness);
	this.dialog.side_one_mate_angle = gui.add_input_double(this.dialog.id, "Side 1 - Mate Angle", NotchMaster.parameters.side_one.mate_angle);
	gui.separator(this.dialog.id);
	this.dialog.side_two_operation = gui.add_radiogroup(this.dialog.id, ['Notch Side 2', 'Slice Side 2', 'Disable']);
	this.dialog.side_two_branch_diameter = gui.add_input_double(this.dialog.id, "Side 2 - Branch Diameter", NotchMaster.parameters.side_two.branch_diameter);
	this.dialog.side_two_header_diameter = gui.add_input_double(this.dialog.id, "Side 2 - Header Diameter", NotchMaster.parameters.side_two.header_diameter);
	this.dialog.side_two_wall_thickness = gui.add_input_double(this.dialog.id, "Side 2 - Wall Thickness", NotchMaster.parameters.side_two.wall_thickness);
	this.dialog.side_two_mate_angle = gui.add_input_double(this.dialog.id, "Side 2 - Mate Angle", NotchMaster.parameters.side_two.mate_angle);
	gui.separator(this.dialog.id);
	this.dialog.post = gui.add_button(this.dialog.id, "Post");
	gui.sameline(this.dialog.id);
	this.dialog.close_button = gui.add_button(this.dialog.id, "Close");
	gui.show(this.dialog.id, false); //Default to hidden
}
NotchMaster.tick = function()
{
	if (gui.get_button(this.dialog.id, this.dialog.close_button))
	{
		this.hide();
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