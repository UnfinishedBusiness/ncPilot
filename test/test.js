var ran_once = false;
function setup()
{
    motion_control.set_port("Arduino");
}
function loop()
{
    //console.log(JSON.stringify(motion_control.get_dro()) + "\n");
    if (motion_control.is_connected() && ran_once == false)
    {
        if (file.open("notch.nc", "r"))
        {
            while(file.lines_available())
            {
                console.log("Sending Line: " + file.read().trim() + "\n");
                motion_control.send(file.read().trim());
            }
            file.close();
            ran_once = true;
        }
    }
}