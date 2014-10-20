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
#include "eastwood/PcxFile.h"
#include "eastwood/WsaFile.h"
#include "eastwood/PalFile.h"
#include "eastwood/ShpFile.h"

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
    infile.open(arcman.find("setup.dip"));
    if(infile.is_open()){
        StringFile str(infile);
        printf("String 5: %s\n", str.getString(5).c_str());
    }
    infile.close();
    infile.open("alibackh.pcx", std::ios_base::in | std::ios_base::binary);
    PcxFile pal(infile);
    infile.close();
    Surface surf = pal.getSurface();
    OStream outfile;
    outfile.open("testing.pcx", std::ios_base::out | std::ios_base::binary);
    if(outfile.is_open()){
            LOG_INFO("Out stream is open");
            //surf.saveBMP(outfile);
            pal.writePcx(outfile);
        }
    outfile.close();
    //outfile.open("testing.cps", std::ios_base::out | std::ios_base::binary);
    //pal.writeCps(outfile);
    //outfile.close();
    #if 0
    infile.open("mouse.shp", std::ios_base::in | std::ios_base::binary);
    if(infile.is_open()){
        LOG_INFO("Opening shp file");
        ShpFile fileformat(infile, pal.getPalette(), SHP_DUNE2);
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
    
    
    //file.open(finfo);
    //StringFile strf(file2);
    //strf.list();
    
    return 0;
}
