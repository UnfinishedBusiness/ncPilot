include("src/MotionControl.js");
include("src/UserInterface.js");
include("src/GcodeViewer.js");
include("src/tools/NotchMaster.js");
function setup()
{
	MotionControl.init();
	UserInterface.init();
	GcodeViewer.init();
	NotchMaster.init();
}
function loop()
{
	//MotionControl.tick();
	//UserInterface.tick();
	//GcodeViewer.tick();
	//sNotchMaster.tick();
}
