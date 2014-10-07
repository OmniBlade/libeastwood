#include "eastwood/CnCFileClass.h"
#include "eastwood/CnCStrFile.h"
#include "eastwood/Log.h"
#include "eastwood/IniFile.h"
#include "eastwood/StdDef.h"
#include "eastwood/Surface.h"
#include "eastwood/CpsFile.h"
#include "eastwood/PcxFile.h"

const char* mixes[] = {"tdtest.mix", "ratest.mix", "rasub.mix"};
const char* strfiles[] = {"conquer.eng", "setup.dip", "redalert.eng"};
const char* inifile[] = { "redalert.ini" };

using namespace eastwood;

int main(int argc, char** argv)
{
    CCFileClass files("./");
    files.init();
    LOG_DEBUG("FileClass initialised");
    files.retrieve(mixes[0]);
    LOG_DEBUG("Retrieving next mix");
    files.retrieve(mixes[1]);
    LOG_DEBUG("Retrieving encrypted sub mix");
    files.retrieve(mixes[2]);
    
    files.open("redalert.ini");
    IniFile inif(files);
    files.close();
    std::string lang = inif.getStringValue("Language", "Language");
    int irq = inif.getIntValue("Sound", "IRQ");
    bool intro = inif.getBoolValue("Intro", "PlayIntro");
    inif.setBoolValue("Intro", "PlayIntro", true);
    LOG_INFO("%s, %d, %d", lang.c_str(), irq, intro);
    
    files.open("testing.ini", FILE_MODE_WRITE);
    inif.SaveChangesTo(files);
    files.close();
    
    files.open("alibackh.pcx");
    LOG_DEBUG("Opened file %s for reading", "alibackh.pcx");
    PcxFile cps(files);
    files.close();
    
    files.open("test1.bmp", FILE_MODE_WRITE);
    LOG_DEBUG("Opened file %s for writing", "test1.bmp");
    cps.saveBMP(files);
    files.close();
    
    return 0;
}
