//
//  MLinkerDB.h
//  CytoMech
//
//  Created by Konstantin Popov on 4/15/14.
//  Copyright (c) 2014 Konstantin Popov. All rights reserved.
//

#ifndef CytoMech_MLinkerDB_h
#define CytoMech_MLinkerDB_h

#include <iostream>
#include <list>
#include "Mcommon.h"
#include "MLinker.h"
#include "MCylinder.h"

///Key to access instance of LinkerDB
class LinkerDBKey {friend class System; LinkerDBKey(); ~LinkerDBKey(); };


/*! An Object Data Base structure will be used as a container for all main objects: Beads, Filament, Linkers and Motors. This structure inherits from std:: list and manage all creations and removing of objects, as well as some stabdart list functions and iterators.
 */

class LinkerDB: private std::list<Linker*>
{
    typedef std::list<Linker*> ldb;
    
public:

    using ldb::size;
    using ldb::begin;
    using ldb::end;
    using ldb::erase;
    
    /// Copying is not allowed
    LinkerDB(const LinkerDB &rhs) = delete;
    
    /// Assignment is not allowed
    LinkerDB& operator=(LinkerDB &rhs) = delete;
    
    static LinkerDB* Instance(LinkerDBKey k);
    
    void CreateLinker(System* ps, Network* pn, Cylinder* pc1, Cylinder* pc2, double k) {
        
        Linker* pl = new Linker(ps, pn, pc1, pc2, k);
        push_back(pl);
        
        
    }

    void RemoveLinker(Linker* pl) {};
    
private:
    static LinkerDB* _instance;
    LinkerDB() {};
    
};



#endif