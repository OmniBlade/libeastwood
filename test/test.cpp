#include "eastwood/IStream.h"
#include "eastwood/ArchiveManager.h"
#include "eastwood/Log.h"
#include "eastwood/IniFile.h"
#include "eastwood/StdDef.h"
#include "eastwood/Surface.h"
#include "eastwood/CpsFile.h"
#include "eastwood/FntFile.h"
#include "eastwood/StringFile.h"
#include "eastwood/OStream.h"
#include "eastwood/PalFile.h"
#include "eastwood/CnCShpFile.h"
#include "eastwood/AudFile.h"

const char* mixes[] = {"tdtest.mix", "ratest.mix", "rasub.mix"};
const char* strfiles[] = {"sole.eng", "conquer.eng", "setup.dip", "redalert.eng"};
const char* inifile[] = { "redalert.ini" };

using namespace eastwood;

int main(int argc, char** argv)
{
    ArchiveManager arcman;
    arcman.indexDir(".");
    arcman.indexMix("ratest.mix", true);
    arcman.indexMix("rasub.mix", true);
    IStream infile;
    
    //string file & mix test
    infile.open(arcman.find("setup.dip"));
    if(infile.is_open()){
        StringFile str(infile);
        printf("String 5: %s\n", str.getString(5).c_str());
    }
    infile.close();
    
    //image test
    infile.open("temperat.pal", std::ios_base::in | std::ios_base::binary);
    PalFile cps(infile);
    infile.close();
    Palette palette = cps.getPalette();
    infile.open("einstein.shp", std::ios_base::in | std::ios_base::binary);
    CnCShpFile shp(infile);
    LOG_DEBUG("Getting tmp tile frame");
    Surface surf = shp[164].getSurface(palette);
    OStream outfile;
    outfile.open("testing.bmp", std::ios_base::out | std::ios_base::binary);
    if(outfile.is_open()){
            LOG_INFO("Out stream is open");
            surf.saveBMP(outfile);
            //pal.writePcx(outfile);
        }
    outfile.close();
    infile.close();
    
    //sound test
    IStream audinfile;
    audinfile.open("abldgin1.aud", std::ios_base::in | std::ios_base::binary);
    if(audinfile.is_open()) {
        LOG_DEBUG("in file is open");
    }
    AudFile aud(audinfile);
    audinfile.close();
    outfile.open("testing.wav", std::ios_base::out | std::ios_base::binary);
    if(outfile.is_open()){
            LOG_INFO("Out stream is open");
            aud.getSound().saveWAV(outfile);
            //pal.writePcx(outfile);
        }
    outfile.close();
    //outfile.open("testing.cps", std::ios_base::out | std::ios_base::binary);
    //pal.writeCps(outfile);
    //outfile.close();
    #if 0
    infile.open("mouse.shp", std::ios_base::in | std::ios_base::binary);
    if(infile.is_open()){
        LOG_INFO("Opening shp file");
        DuneShpFile fileformat(infile, pal.getPalette(), SHP_DUNE2);
        LOG_INFO("Shp has %d fames", fileformat.size());
        OStream outfile;
        outfile.open("testing.bmp", std::ios_base::out | std::ios_base::binary);
        //Format20 in einstein 13 164 not working
        Surface surface = fileformat.getSurface(13);
        if(outfile.is_open()){
            LOG_INFO("Out stream is open");
            surface.saveBMP(outfile);
        }
    } else {
        LOG_DEBUG("File not open");
    }
    infile.close();
    #endif
    
    return 0;
}
