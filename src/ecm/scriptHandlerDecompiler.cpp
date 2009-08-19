/* DST  -  Dune 2 Script Tools
 *  
 * Copyright (C) 2009 segra		<segra@strobs.com>

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *  
 * 
 * $Id: scriptHandlerDecompiler.cpp 25 2009-04-28 09:58:25Z segra $
 * 
 */

#include <iostream>
#include <iomanip>

#include "scriptHandler.h"
#include "scriptHandlerDecompiler.h"

#include "ObjectNames.h"

// Unit GetDetail Function DetailNames
static const char *nameUnitDetails[] = {
    "HitPointRepairCalc?",   
    "BuildingTypeIndex",     
    "WeaponRange",           
    "IndexGet",              
    "Field64Get",            
    "AttackObjectIndexTypeGet",                   
    "Return",
    "TypeIndex",             
    "IndexGetAsObject",      
    "Field6BGet",                 
    "loc_27562",          
    "Field49AndField4BGet",                 
    "WeaponCoolDown?",                 
    "Field36And4h",               
    "HouseIDGet",            
    "loc_275C1",                 
    "TurretGet?",     
    "loc_2752F",                 
    "DoesTurretRotate",          
    "CheckIfHuman"              
};

_scriptHandlerDecompiler::_scriptHandlerDecompiler(const char *fileName) : _scriptHandler(fileName) {
    std::string	sourceFilename = std::string(_fileName), targetFilename;
    size_t posPeriod = sourceFilename.find(".");

    // Prepare output filename
    if(posPeriod == std::string::npos)
	posPeriod = sourceFilename.length() - 1;

    targetFilename = sourceFilename.substr(0, posPeriod);
    targetFilename.append(".txt");

    _scriptLastPush = 0;
    _opcodesExecute = NULL;

    opcodesSetup();

    // Open target file
    _destinationFile.open(targetFilename.c_str(), std::ios::out);
}

_scriptHandlerDecompiler::~_scriptHandlerDecompiler() {
    _destinationFile.close();
}

bool _scriptHandlerDecompiler::scriptLoad() {
    std::ifstream fileScript;
    size_t scriptSize;


    fileScript.open(_fileName, std::ios::in | std::ios::binary);

    if(fileScript.is_open() == false)
	return false;

    // Read File Size
    fileScript.seekg(0, std::ios::end);
    scriptSize = fileScript.tellg();
    fileScript.seekg(std::ios::beg);

    // Load file into _scriptBuffer
    _scriptBuffer = new uint8_t[scriptSize];
    if(fileScript.read((char*) _scriptBuffer, scriptSize) == false)
	return false;

    fileScript.close();

    return true;
}

bool _scriptHandlerDecompiler::headerRead() {
    uint16_t *buffer = (uint16_t*) (_scriptBuffer + 0x12);

    // Number of script functions
    _pointerCount = htobe16(*buffer) / 2;
    buffer++;

    _headerPointers = new uint16_t[_pointerCount];

    for(size_t ptrCount = 0; ptrCount < _pointerCount; ptrCount++) {
	_headerPointers[ptrCount] = htobe16(*buffer);
	buffer++;
    }

    // Skip 'DATA' tag
    buffer += 2;	// 2 Words

    // Not sure if this uint16_t is actually part of a DWORD, or 2 useless uint8_ts?
    buffer++;

    // Size of the upcoming script data
    _scriptSize = htobe16(*buffer);
    buffer++;

    // Start of script data
    _scriptStart = (uint8_t*) buffer;

    if(_pointerCount == sizeof(nameHouses)/sizeof(*nameHouses)) {
	_destinationFile << "[House]" << std::endl;
	_scriptType	= _scriptHOUSE;
	_objectNames	= nameHouses;
	opcodesHousesSetup();
    }

    if(_pointerCount == sizeof(nameStructures)/sizeof(*nameStructures)) {
	_destinationFile << "[Build]" << std::endl;
	_scriptType	= _scriptBUILD;
	_objectNames	= nameStructures;
	opcodesBuildingsSetup();
    }

    if(_pointerCount == sizeof(nameUnits)/sizeof(*nameUnits)) {
	_destinationFile << "[Unit]" << std::endl;
	_scriptType	= _scriptUNIT;
	_objectNames	= nameUnits;
	opcodesUnitsSetup();
    }

    return true;
}

bool _scriptHandlerDecompiler::scriptNextStart() {
    bool found = false;

    // Loop through each pointer and write the section names out
    for(int count = 0; count < _pointerCount; count++) {

	// In TEAM.EMC for example, two objects use the same script
	if(_scriptPos == (uint16_t) _headerPointers[count]) {
	    if(found==false)
		_destinationFile << std::endl;

	    // Write the section name in square brackets
	    _destinationFile << "[" << _objectNames[count] << "]" << std::endl;
	    found = true;
	}
    }

    return found;
}

bool _scriptHandlerDecompiler::execute() {

    std::cout << "Preprocessing " << _fileName << std::endl;

    // Load the script into a _scriptBuffer, then read the header information
    scriptLoad();
    headerRead();

    // Map out all labels
    _modePreProcess = true;
    if(scriptDecompile() == false)
	return false;

    // Decompile the script 
    _modePreProcess = false;

    _destinationFile << "[General]" << std::endl;

    std::cout << "Decompiling....." << std::endl;
    return scriptDecompile();
}

bool _scriptHandlerDecompiler::scriptDecompile() {

    _lineCount		= 0;
    _opcodeCurrent	= 0;
    _scriptData		= _scriptDataNext = 0;
    _scriptPtr		= NULL;
    _scriptPtrEnd	= NULL;
    _scriptPos		= 0;
    _stackCount		= 0xF;

    _scriptPos		= 0;
    _scriptPtr		= (uint16_t*) _scriptStart;
    _scriptPtrEnd	= (uint16_t*) (_scriptStart + _scriptSize);

    while(_scriptPtr <  _scriptPtrEnd) {

	if(!_modePreProcess) {
	    scriptNextStart();

	    // Check if label location, print label if so
	    if(scriptLabel(_scriptPos) != (size_t)-1)
		_destinationFile << "l" << _scriptPos << ":" << std::endl;
	}

	_scriptDataNext = 0;
	_scriptData = htobe16(*_scriptPtr);
	_scriptPtr++;
	_scriptPos++;

	// Keep the opcode
	_opcodeCurrent = _scriptData >> 8;
	_opcodeCurrent &= 0x1F;

	if(_scriptData & 0x8000) {
	    // Opcode uses 13 bits
	    _opcodeCurrent = 0;
	    _scriptData &= 0x7FFF;
	} else
	    // Opcode only requires 1 uint8_t
	    if(_scriptData & 0x4000) {
		_scriptData &= 0xFF;
	    } else 	
		// Opcode uses the next WORD, grab it
		if(_scriptData & 0x2000) {
		    _scriptDataNext = htobe16(*_scriptPtr);
		    _scriptPtr++;
		    _scriptPos++;
		}


	    // Print opcode
	    if(!_modePreProcess)
		_destinationFile << std::setw(20) << std::left << _opcodes[_opcodeCurrent].description;

	    // Excute opcode
	    (this->*_opcodes[_opcodeCurrent].function)();

	    //_destinationFile  << setw(20) << " ";
	    //_destinationFile  << hex << uppercase << "S: 0x" << _stackCount << std::endl;
	    if(!_modePreProcess)
		_destinationFile << std::endl;

	    _lineCount++;
    }

    return true;
}

void _scriptHandlerDecompiler::o_goto() {
    size_t labelPos = scriptLabel(_scriptData);

    if(!_modePreProcess) {

	if(labelPos == (size_t)-1)
	    dataPrint(_scriptData);
	else
	    _destinationFile << "l" << _scriptLabels[labelPos]._scriptPos;

    } else {
	if(labelPos == (size_t)-1)
	    scriptLabelAdd("", _scriptData);
    }
}

void _scriptHandlerDecompiler::o_setreturn() {
    if(_scriptDataNext)
	dataPrint(_scriptDataNext);
    else
	dataPrint(_scriptData);
}

void _scriptHandlerDecompiler::o_pushOp() {
    uint16_t data = _scriptData;
    if(_scriptData == 0)
	data = _scriptDataNext;

    _stackCount--;

    dataPrint(data);
}

void _scriptHandlerDecompiler::o_push() {
    _stackCount--;

    if(_scriptDataNext) {
	_scriptLastPush = _scriptDataNext;
	dataPrint(_scriptDataNext);
    } else {
	_scriptLastPush = _scriptData;
	dataPrint(_scriptData);
    }
}

void _scriptHandlerDecompiler::o_pushWord() {
    o_push();
}

void _scriptHandlerDecompiler::o_pushreg() {
    _stackCount--;

    if(_scriptDataNext)
	dataPrint(_scriptDataNext);
    else
	dataPrint(_scriptData);
}

void _scriptHandlerDecompiler::o_pushframeMinArg() {
    _stackCount--;
    _stackCount--;
    if(_scriptDataNext)
	dataPrint(_scriptDataNext);
    else
	dataPrint(_scriptData);
}

void _scriptHandlerDecompiler::o_pushframePluArg() {
    _stackCount--;
    _stackCount--;
    if(_scriptDataNext)
	dataPrint(_scriptDataNext);
    else
	dataPrint(_scriptData);
}

void _scriptHandlerDecompiler::o_popret() {
    if(_scriptData == 1) {
	if(!_modePreProcess)
	    _destinationFile << " (Return)";
	return;

    } else 
	_stackCount++;

    _stackCount++;
}

void _scriptHandlerDecompiler::o_popreg() {
    _stackCount++;

    if(_scriptDataNext)
	dataPrint(_scriptDataNext);
    else
	dataPrint(_scriptData);
}

void _scriptHandlerDecompiler::o_popframeMinArg() {
    _stackCount++;
    _stackCount++;
    if(_scriptDataNext)
	dataPrint(_scriptDataNext);
    else
	dataPrint(_scriptData);
}

void _scriptHandlerDecompiler::o_popframePluArg() {
    _stackCount++;
    _stackCount++;
    if(_scriptDataNext)
	dataPrint(_scriptDataNext);
    else
	dataPrint(_scriptData);
}

void _scriptHandlerDecompiler::o_spadd() {
    dataPrint(_scriptData);
    _stackCount += (_scriptData & 0xF);
}

void _scriptHandlerDecompiler::o_spsub() {
    dataPrint(_scriptData);
    _stackCount -= (_scriptData & 0xF);
}

void _scriptHandlerDecompiler::o_execute() {

    if(!_modePreProcess)
	_destinationFile << std::left << _opcodesExecute[ _scriptData ].description << " ";

    (this->*_opcodesExecute[ _scriptData ].function)();
}

void _scriptHandlerDecompiler::o_ifnotgoto() {
    size_t labelPos;

    if(_scriptDataNext) {
	labelPos = scriptLabel(_scriptDataNext & 0x7FFF);

	if(_modePreProcess)  {
	    if(labelPos == (size_t)-1)
		scriptLabelAdd("", _scriptDataNext & 0x7FFF);

	} else {
	    if(labelPos == (size_t)-1)
		dataPrint(_scriptDataNext & 0x7FFF);
	    else
		_destinationFile << "l" << _scriptLabels[labelPos]._scriptPos;
	}

    } else {
	labelPos = scriptLabel(_scriptData);

	if(_modePreProcess) {
	    if(labelPos == (size_t)-1)
		scriptLabelAdd("", _scriptData);
	} else {
	    if(labelPos == (size_t)-1)
		dataPrint(_scriptData);
	    else
		_destinationFile << "l" << _scriptLabels[labelPos]._scriptPos;
	}
    }
}

void _scriptHandlerDecompiler::o_negate() {
    dataPrint(_scriptData);
}

void _scriptHandlerDecompiler::o_evaluate() {
    if(!_modePreProcess)
	_destinationFile << _opcodesEvaluate[ _scriptData ].description;

    (this->*_opcodesEvaluate[ _scriptData ].function)();
}

void _scriptHandlerDecompiler::o_return() {

}


void _scriptHandlerDecompiler::o_execute_Unit_GetDetail() {
    if(!_modePreProcess)
	_destinationFile << "(" << nameUnitDetails[_scriptLastPush] << ")";
}
