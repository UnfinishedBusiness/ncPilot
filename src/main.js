include("src/MotionControl.js");
include("src/UserInterface.js");
include("src/GcodeViewer.js");
function setup()
{
	MotionControl.init();
	UserInterface.init();
	GcodeViewer.init();
}
function loop()
{
	MotionControl.tick();
	UserInterface.tick();
	GcodeViewer.tick();
}
