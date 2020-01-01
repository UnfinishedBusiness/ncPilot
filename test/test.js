var ran_once = false;
function setup()
{
    if (serial.open("COM3", 115200))
    {
        console.log("Opened serial!\n");
    }
    else
    {
        console.log("Could not open port!\n");
        exit(1);
    }
}
function loop()
{
    if (serial.is_open())
    {
        serial.write("?");
    }
}