/* 
 * File:   ccstrfile.h
 * Author: aidan
 *
 * Created on 06 September 2014, 23:35
 */

#ifndef CNCSTRFILE_H
#define	CNCSTRFILE_H

#include "eastwood/StdDef.h"
#include "eastwood/CnCFileClass.h"
#include "eastwood/Strings.h"

namespace eastwood {

enum Language{
	LANGUAGE_ENGLISH     = 0,
	LANGUAGE_FRENCH      = 1,
	LANGUAGE_GERMAN      = 2,
	LANGUAGE_ITALIAN     = 3,
	LANGUAGE_SPANISH     = 4,

	LANGUAGE_MAX         = 5,
	LANGUAGE_INVALID     = 0xFF
};

static const std::string g_lang_suffixes[] = {"ENG", "FRE", "GER", "ITA", "SPA"};

class CCStringFile
{
public:
    CCStringFile(CCFileClass& file);
    std::string getString(uint32_t index) const{
        return _strings.at(index);
    }
    uint16_t size() const throw() {
	    return _strings.size();
	}
    void list();
private:
    std::vector<std::string> _strings;
};

} //eastwood

#endif	/* CNCSTRFILE_H */
