//
//  InitializerImpl.cpp
//  Cyto
//
//  Created by James Komianos on 7/30/14.
//  Copyright (c) 2014 University of Maryland. All rights reserved.
//

#include "InitializerImpl.h"
#include "ChemCallbacks.h"
#include "Bead.h"

#include "SystemParameters.h"
#include "MathFunctions.h"

using namespace mathfunc;

void InitializerImpl::generateFilamentReactionTemplates(ChemistrySpeciesAndReactions& chemSR) {
    
    ///set up reaction templates
    for(auto &r: chemSR.polymerizationReactions) {
        
        std::vector<std::tuple<int, SpeciesType>> reactantTemplate;
        std::vector<std::tuple<int, SpeciesType>> productTemplate;
        FilamentReactionDirection d;
        
        std::vector<std::string> reactants = std::get<0>(r);
        std::vector<std::string> products = std::get<1>(r);
        ///read strings, and look up type
        
        ///Checks on number of reactants, products
        if(reactants.size() != 2 || products.size() != 3) {
            std::cout << "Invalid polymerization reaction. Exiting." << std::endl;
            exit(EXIT_FAILURE);
        }
        
        ///FIRST SPECIES MUST BE BULK OR DIFFUSING
        auto reactant = reactants[0];
        if(reactant.find("BULK") != std::string::npos) {
            
            ///Look up species, make sure in list
            std::string name = reactant.substr(0, reactant.find(":"));
            auto it = std::find_if(chemSR.speciesBulk.begin(), chemSR.speciesBulk.end(),
                                   [name](std::tuple<std::string, int> element) { return std::get<0>(element) == name ? true : false; });
                                       
            if(it == chemSR.speciesBulk.end()) {
                std::cout << "A bulk species that was included in a reaction was not initialized. Exiting." << std::endl;
                exit(EXIT_FAILURE);
            }
            reactantTemplate.push_back(std::tuple<int, SpeciesType>(SpeciesNamesDB::Instance()->stringToInt(name), SpeciesType::BULK));
        }
        
        else if(reactant.find("DIFFUSING") != std::string::npos) {
            
            ///Look up species, make sure in list
            std::string name = reactant.substr(0, reactant.find(":"));
            auto it = std::find_if(chemSR.speciesDiffusing.begin(), chemSR.speciesDiffusing.end(),
                                   [name](std::tuple<std::string, int, double> element) { return std::get<0>(element) == name ? true : false; });
            if(it == chemSR.speciesDiffusing.end()) {
                std::cout << "A diffusing species that was included in a reaction was not initialized. Exiting." << std::endl;
                exit(EXIT_FAILURE);
            }
            reactantTemplate.push_back(std::tuple<int, SpeciesType>(SpeciesNamesDB::Instance()->stringToInt(name), SpeciesType::DIFFUSING));
        }
        else {
            std::cout << "First species listed in a polymerization reaction must be either bulk or diffusing. Exiting." << std::endl;
            exit(EXIT_FAILURE);
        }
        
        ///SECOND SPECIES MUST BE PLUS OR MINUS END
        reactant = reactants[1];
        if(reactant.find("PLUSEND") != std::string::npos) {
            
            ///look up species, make sure in list
            std::string name = reactant.substr(0, reactant.find(":"));
            auto it = std::find(_speciesPlusEnd.begin(), _speciesPlusEnd.end(), name);
            int position = 0;
            
            if(it != _speciesPlusEnd.end()) {
                
                ///get position of iterator
                position = std::distance(_speciesPlusEnd.begin(), it);
                reactantTemplate.push_back(std::tuple<int, SpeciesType>(position, SpeciesType::PLUSEND));
                
                d = FilamentReactionDirection::FORWARD;
            }
            else {
                std::cout << "A plus end species that was included in a reaction was not initialized. Exiting." << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        
        else if(reactant.find("MINUSEND") != std::string::npos) {
            
            ///look up species, make sure in list
            std::string name = reactant.substr(0, reactant.find(":"));
            auto it = std::find(_speciesMinusEnd.begin(), _speciesMinusEnd.end(), name);
            int position = 0;
            
            if(it != _speciesMinusEnd.end()) {
                
                ///get position of iterator
                position = std::distance(_speciesMinusEnd.begin(), it);
                reactantTemplate.push_back(std::tuple<int, SpeciesType>(position, SpeciesType::MINUSEND));
                
                d = FilamentReactionDirection::BACKWARD;
            }
            else {
                std::cout << "A minus species that was included in a reaction was not initialized. Exiting." << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        else {
            
            std::cout << "Second species listed in a polymerization reaction must be either plusend or minusend. Exiting." << std::endl;
            exit(EXIT_FAILURE);
            
        }
        
        ///FIRST PRODUCT SPECIES MUST BE FILAMENT SPECIES
        auto product = products[0];
        if(product.find("FILAMENT") != std::string::npos) {
            
            ///look up species, make sure in list
            std::string name = product.substr(0, product.find(":"));
            auto it = std::find(_speciesFilament.begin(), _speciesFilament.end(), name);
            int position = 0;
            
            if(it != _speciesFilament.end()) {
                
                ///get position of iterator
                position = std::distance(_speciesFilament.begin(), it);
                productTemplate.push_back(std::tuple<int, SpeciesType>(position, SpeciesType::FILAMENT));
            }
            else {
                std::cout << "A filament species that was included in a reaction was not initialized. Exiting." << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        else {
            std::cout << "Third species listed in a polymerization reaction must be filament. Exiting." << std::endl;
            exit(EXIT_FAILURE);
        }
        
        ///SECOND PRODUCT SPECIES MUST BE BOUND
        product = products[1];
        ///read strings, and look up type
        if(product.find("BOUND") != std::string::npos) {
            
            ///look up species, make sure in list
            std::string name = product.substr(0, product.find(":"));
            auto it = std::find(_speciesBound.begin(), _speciesBound.end(), name);
            int position = 0;
            
            if(it != _speciesBound.end()) {
                
                ///get position of iterator
                position = std::distance(_speciesBound.begin(), it);
                productTemplate.push_back(std::tuple<int, SpeciesType>(position, SpeciesType::BOUND));
            }
            else {
                std::cout << "A bound species that was included in a reaction was not initialized. Exiting." << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        else{
            std::cout << "Fourth species listed in a polymerization reaction must be bound. Exiting." << std::endl;
            exit(EXIT_FAILURE);
            
        }
        
        ///THIRD PRODUCT SPECIES MUST BE PLUS OR MINUS END
        product = products[2];
        ///read strings, and look up type
        if(product.find("PLUSEND") != std::string::npos) {
            
            ///look up species, make sure in list
            std::string name = product.substr(0, product.find(":"));
            auto it = std::find(_speciesPlusEnd.begin(), _speciesPlusEnd.end(), name);
            int position = 0;
            
            if(it != _speciesPlusEnd.end()) {
                
                ///get position of iterator
                position = std::distance(_speciesPlusEnd.begin(), it);
                productTemplate.push_back(std::tuple<int, SpeciesType>(position, SpeciesType::PLUSEND));
            }
            else {
                std::cout << "A plusend species that was included in a reaction was not initialized. Exiting." << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        
        else if(product.find("MINUSEND") != std::string::npos) {
            
            ///look up species, make sure in list
            std::string name = product.substr(0, product.find(":"));
            auto it = std::find(_speciesMinusEnd.begin(), _speciesMinusEnd.end(), name);
            int position = 0;
            
            if(it != _speciesMinusEnd.end()) {
                
                ///get position of iterator
                position = std::distance(_speciesMinusEnd.begin(), it);
                productTemplate.push_back(std::tuple<int, SpeciesType>(position, SpeciesType::MINUSEND));
            }
            else {
                std::cout << "A plusend species that was included in a reaction was not initialized. Exiting." << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        else {
            std::cout << "Fifth species listed in a polymerization reaction must be either plusend or minusend. Exiting." << std::endl;
            exit(EXIT_FAILURE);
        }
        
        ///Add polymerization template
        if(d == FilamentReactionDirection::FORWARD)
            _filamentReactionTemplates.emplace_back(
                new PolymerizationPlusEndTemplate(reactantTemplate, productTemplate, std::get<2>(r)));
        else
            _filamentReactionTemplates.emplace_back(
                new PolymerizationMinusEndTemplate(reactantTemplate, productTemplate, std::get<2>(r)));
    }
    
    
    
    ///set up reaction templates
    for(auto &r: chemSR.depolymerizationReactions) {
        
        std::vector<std::tuple<int, SpeciesType>> reactantTemplate;
        std::vector<std::tuple<int, SpeciesType>> productTemplate;
        FilamentReactionDirection d;
        
        std::vector<std::string> reactants = std::get<0>(r);
        std::vector<std::string> products = std::get<1>(r);
        ///read strings, and look up type
        
        ///Checks on number of reactants, products
        if(reactants.size() != 3 || products.size() != 2) {
            std::cout << "Invalid depolymerization reaction. Exiting." << std::endl;
            exit(EXIT_FAILURE);
        }

        ///FIRST REACTANT SPECIES MUST BE FILAMENT SPECIES
        auto reactant = reactants[0];
        if(reactant.find("FILAMENT") != std::string::npos) {
            
            ///look up species, make sure in list
            std::string name = reactant.substr(0, reactant.find(":"));
            auto it = std::find(_speciesFilament.begin(), _speciesFilament.end(), name);
            int position = 0;
            
            if(it != _speciesFilament.end()) {
                
                ///get position of iterator
                position = std::distance(_speciesFilament.begin(), it);
                reactantTemplate.push_back(std::tuple<int, SpeciesType>(position, SpeciesType::FILAMENT));
            }
            else {
                std::cout << "A filament species that was included in a reaction was not initialized. Exiting." << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        else {
            std::cout << "First species listed in a depolymerization reaction must be filament. Exiting." << std::endl;
            exit(EXIT_FAILURE);
        }
        
        ///SECOND REACTANT SPECIES MUST BE BOUND
        reactant = reactants[1];
        ///read strings, and look up type
        if(reactant.find("BOUND") != std::string::npos) {
            
            ///look up species, make sure in list
            std::string name = reactant.substr(0, reactant.find(":"));
            auto it = std::find(_speciesBound.begin(), _speciesBound.end(), name);
            int position = 0;
            
            if(it != _speciesBound.end()) {
                
                ///get position of iterator
                position = std::distance(_speciesBound.begin(), it);
                reactantTemplate.push_back(std::tuple<int, SpeciesType>(position, SpeciesType::BOUND));
            }
            else {
                std::cout << "A bound species that was included in a reaction was not initialized. Exiting." << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        else{
            std::cout << "Second species listed in a depolymerization reaction must be bound. Exiting." << std::endl;
            exit(EXIT_FAILURE);
            
        }
        
        ///THIRD REACTANT SPECIES MUST BE PLUS OR MINUS END
        reactant = reactants[2];
        ///read strings, and look up type
        if(reactant.find("PLUSEND") != std::string::npos) {
            
            ///look up species, make sure in list
            std::string name = reactant.substr(0, reactant.find(":"));
            auto it = std::find(_speciesPlusEnd.begin(), _speciesPlusEnd.end(), name);
            int position = 0;
            
            if(it != _speciesPlusEnd.end()) {
                
                ///get position of iterator
                position = std::distance(_speciesPlusEnd.begin(), it);
                reactantTemplate.push_back(std::tuple<int, SpeciesType>(position, SpeciesType::PLUSEND));
                
                d = FilamentReactionDirection::BACKWARD;
            }
            else {
                std::cout << "A plusend species that was included in a reaction was not initialized. Exiting." << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        
        else if(reactant.find("MINUSEND") != std::string::npos) {
            
            ///look up species, make sure in list
            std::string name = reactant.substr(0, reactant.find(":"));
            auto it = std::find(_speciesMinusEnd.begin(), _speciesMinusEnd.end(), name);
            int position = 0;
            
            if(it != _speciesMinusEnd.end()) {
                
                ///get position of iterator
                position = std::distance(_speciesMinusEnd.begin(), it);
                reactantTemplate.push_back(std::tuple<int, SpeciesType>(position, SpeciesType::MINUSEND));
                d = FilamentReactionDirection::FORWARD;
            }
            else {
                std::cout << "A minusend species that was included in a reaction was not initialized. Exiting." << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        else {
            std::cout << "Third species listed in a depolymerization reaction must be either plusend or minusend. Exiting." << std::endl;
            exit(EXIT_FAILURE);
        }
        
        
        ///FIRST PRODUCT SPECIES MUST BE BULK OR DIFFUSING
        auto product = products[0];
        if(product.find("BULK") != std::string::npos) {
            
            ///Look up species, make sure in list
            std::string name = product.substr(0, product.find(":"));
            auto it = std::find_if(chemSR.speciesBulk.begin(), chemSR.speciesBulk.end(),
                                   [name](std::tuple<std::string, int> element) { return std::get<0>(element) == name ? true : false; });
            
            if(it == chemSR.speciesBulk.end()) {
                std::cout << "A bulk species that was included in a reaction was not initialized. Exiting." << std::endl;
                exit(EXIT_FAILURE);
            }
            productTemplate.push_back(std::tuple<int, SpeciesType>(SpeciesNamesDB::Instance()->stringToInt(name), SpeciesType::BULK));
        }
        
        else if(product.find("DIFFUSING") != std::string::npos) {
            
            ///Look up species, make sure in list
            std::string name = product.substr(0, product.find(":"));
            auto it = std::find_if(chemSR.speciesDiffusing.begin(), chemSR.speciesDiffusing.end(),
                                   [name](std::tuple<std::string, int, double> element) { return std::get<0>(element) == name ? true : false; });
            if(it == chemSR.speciesDiffusing.end()) {
                std::cout << "A diffusing species that was included in a reaction was not initialized. Exiting." << std::endl;
                exit(EXIT_FAILURE);
            }
            productTemplate.push_back(std::tuple<int, SpeciesType>(SpeciesNamesDB::Instance()->stringToInt(name), SpeciesType::DIFFUSING));
        }
        else {
            std::cout << "Fourth species listed in a depolymerization reaction must be either bulk or diffusing. Exiting." << std::endl;
            exit(EXIT_FAILURE);
        }
        
        ///SECOND PRODUCT SPECIES MUST BE PLUS OR MINUS END
        product = products[1];
        if(product.find("PLUSEND") != std::string::npos) {
            
            ///look up species, make sure in list
            std::string name = product.substr(0, product.find(":"));
            auto it = std::find(_speciesPlusEnd.begin(), _speciesPlusEnd.end(), name);
            int position = 0;
            
            if(it != _speciesPlusEnd.end()) {
                
                ///get position of iterator
                position = std::distance(_speciesPlusEnd.begin(), it);
                productTemplate.push_back(std::tuple<int, SpeciesType>(position, SpeciesType::PLUSEND));
            }
            else {
                std::cout << "A plus end species that was included in a reaction was not initialized. Exiting." << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        
        else if(product.find("MINUSEND") != std::string::npos) {
            
            ///look up species, make sure in list
            std::string name = product.substr(0, product.find(":"));
            auto it = std::find(_speciesMinusEnd.begin(), _speciesMinusEnd.end(), name);
            int position = 0;
            
            if(it != _speciesMinusEnd.end()) {
                
                ///get position of iterator
                position = std::distance(_speciesMinusEnd.begin(), it);
                productTemplate.push_back(std::tuple<int, SpeciesType>(position, SpeciesType::MINUSEND));
            }
            else {
                std::cout << "A minus species that was included in a reaction was not initialized. Exiting." << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        else {
            std::cout << "Fifth species listed in a depolymerization reaction must be either plusend or minusend. Exiting." << std::endl;
            exit(EXIT_FAILURE);
        }
        
        ///Add depolymerization template
        if(d == FilamentReactionDirection::FORWARD)
            _filamentReactionTemplates.emplace_back(
                     new DepolymerizationMinusEndTemplate(reactantTemplate, productTemplate, std::get<2>(r)));
        else
            _filamentReactionTemplates.emplace_back(
                     new DepolymerizationPlusEndTemplate(reactantTemplate, productTemplate, std::get<2>(r)));
    }

    
    ///set up reaction templates
    for(auto &r: chemSR.bindingReactions) {
        
        std::vector<std::tuple<int, SpeciesType>> reactantTemplate;
        std::vector<std::tuple<int, SpeciesType>> productTemplate;
        
        std::vector<std::string> reactants = std::get<0>(r);
        std::vector<std::string> products = std::get<1>(r);
        ///read strings, and look up type
        
        ///Checks on number of reactants, products
        if(reactants.size() != 2 || products.size() != 1) {
            std::cout << "Invalid binding reaction. Exiting." << std::endl;
            exit(EXIT_FAILURE);
        }
        
        ///FIRST REACTANT SPECIES MUST BE BOUND
        auto reactant = reactants[0];
        ///read strings, and look up type
        if(reactant.find("BOUND") != std::string::npos) {
            
            ///look up species, make sure in list
            std::string name = reactant.substr(0, reactant.find(":"));
            auto it = std::find(_speciesBound.begin(), _speciesBound.end(), name);
            int position = 0;
            
            if(it != _speciesBound.end()) {
                
                ///get position of iterator
                position = std::distance(_speciesBound.begin(), it);
                reactantTemplate.push_back(std::tuple<int, SpeciesType>(position, SpeciesType::BOUND));
            }
            else {
                std::cout << "A bound species that was included in a reaction was not initialized. Exiting." << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        else{
            std::cout << "First species listed in a binding reaction must be bound. Exiting." << std::endl;
            exit(EXIT_FAILURE);
        }
        
        ///SECOND REACTANT SPECIES MUST BE BULK OR DIFFUSING
        reactant = reactants[1];
        if(reactant.find("BULK") != std::string::npos) {
            
            ///Look up species, make sure in list
            std::string name = reactant.substr(0, reactant.find(":"));
            auto it = std::find_if(chemSR.speciesBulk.begin(), chemSR.speciesBulk.end(),
                                   [name](std::tuple<std::string, int> element) { return std::get<0>(element) == name ? true : false; });
            
            if(it == chemSR.speciesBulk.end()) {
                std::cout << "A bulk species that was included in a reaction was not initialized. Exiting." << std::endl;
                exit(EXIT_FAILURE);
            }
            reactantTemplate.push_back(std::tuple<int, SpeciesType>(SpeciesNamesDB::Instance()->stringToInt(name), SpeciesType::BULK));
        }
        
        else if(reactant.find("DIFFUSING") != std::string::npos) {
            
            ///Look up species, make sure in list
            std::string name = reactant.substr(0, reactant.find(":"));
            auto it = std::find_if(chemSR.speciesDiffusing.begin(), chemSR.speciesDiffusing.end(),
                                   [name](std::tuple<std::string, int, double> element) { return std::get<0>(element) == name ? true : false; });
            if(it == chemSR.speciesDiffusing.end()) {
                std::cout << "A diffusing species that was included in a reaction was not initialized. Exiting." << std::endl;
                exit(EXIT_FAILURE);
            }
            reactantTemplate.push_back(std::tuple<int, SpeciesType>(SpeciesNamesDB::Instance()->stringToInt(name), SpeciesType::DIFFUSING));
        }
        else {
            std::cout << "Second species listed in a binding reaction must be either bulk or diffusing. Exiting." << std::endl;
            exit(EXIT_FAILURE);
        }

        
        ///FIRST PRODUCT SPECIES MUST BE BOUND
        auto product = products[0];
        ///read strings, and look up type
        if(product.find("BOUND") != std::string::npos) {
            
            ///look up species, make sure in list
            std::string name = product.substr(0, product.find(":"));
            auto it = std::find(_speciesBound.begin(), _speciesBound.end(), name);
            int position = 0;
            
            if(it != _speciesBound.end()) {
                
                ///get position of iterator
                position = std::distance(_speciesBound.begin(), it);
                productTemplate.push_back(std::tuple<int, SpeciesType>(position, SpeciesType::BOUND));
            }
            else {
                std::cout << "A bound species that was included in a reaction was not initialized. Exiting." << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        else{
            std::cout << "Third species listed in a binding reaction must be bound. Exiting." << std::endl;
            exit(EXIT_FAILURE);
        }
    
    
        ///add reaction
        _filamentReactionTemplates.emplace_back(new BasicBindingTemplate(reactantTemplate, productTemplate, std::get<2>(r)));
    
    }

    ///set up reaction templates
    for(auto &r: chemSR.unbindingReactions) {
        
        std::vector<std::tuple<int, SpeciesType>> reactantTemplate;
        std::vector<std::tuple<int, SpeciesType>> productTemplate;
        
        std::vector<std::string> reactants = std::get<0>(r);
        std::vector<std::string> products = std::get<1>(r);
        ///read strings, and look up type
        
        ///Checks on number of reactants, products
        if(reactants.size() != 1 || products.size() != 2) {
            std::cout << "Invalid unbinding reaction. Exiting." << std::endl;
            exit(EXIT_FAILURE);
        }
        
        ///FIRST REACTANT SPECIES MUST BE BOUND, LINKER, MOTOR
        auto reactant = reactants[0];
        ///read strings, and look up type
        if(reactant.find("BOUND") != std::string::npos) {
            
            ///look up species, make sure in list
            std::string name = reactant.substr(0, reactant.find(":"));
            auto it = std::find(_speciesBound.begin(), _speciesBound.end(), name);
            int position = 0;
            
            if(it != _speciesBound.end()) {
                
                ///get position of iterator
                position = std::distance(_speciesBound.begin(), it);
                reactantTemplate.push_back(std::tuple<int, SpeciesType>(position, SpeciesType::BOUND));
            }
            else {
                std::cout << "A bound species that was included in a reaction was not initialized. Exiting." << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        else if(reactant.find("LINKER") != std::string::npos) {
            
            ///look up species, make sure in list
            std::string name = reactant.substr(0, reactant.find(":"));
            auto it = std::find(_speciesLinker.begin(), _speciesLinker.end(), name);
            int position = 0;
            
            if(it != _speciesLinker.end()) {
                
                ///get position of iterator
                position = std::distance(_speciesLinker.begin(), it);
                reactantTemplate.push_back(std::tuple<int, SpeciesType>(position, SpeciesType::LINKER));
            }
            else {
                std::cout << "A linker species that was included in a reaction was not initialized. Exiting." << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        else if(reactant.find("MOTOR") != std::string::npos) {
            
            ///look up species, make sure in list
            std::string name = reactant.substr(0, reactant.find(":"));
            auto it = std::find(_speciesMotor.begin(), _speciesMotor.end(), name);
            int position = 0;
            
            if(it != _speciesMotor.end()) {
                
                ///get position of iterator
                position = std::distance(_speciesMotor.begin(), it);
                reactantTemplate.push_back(std::tuple<int, SpeciesType>(position, SpeciesType::MOTOR));
            }
            else {
                std::cout << "A motor species that was included in a reaction was not initialized. Exiting." << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        else{
            std::cout << "First species listed in an unbinding reaction must be bound, linker, or motor. Exiting." << std::endl;
            exit(EXIT_FAILURE);
        }
        
        ///FIRST PRODUCT SPECIES MUST BE BULK OR DIFFUSING
        auto product = products[0];
        if(product.find("BULK") != std::string::npos) {
            
            ///Look up species, make sure in list
            std::string name = product.substr(0, product.find(":"));
            auto it = std::find_if(chemSR.speciesBulk.begin(), chemSR.speciesBulk.end(),
                                   [name](std::tuple<std::string, int> element) { return std::get<0>(element) == name ? true : false; });
            
            if(it == chemSR.speciesBulk.end()) {
                std::cout << "A bulk species that was included in a reaction was not initialized. Exiting." << std::endl;
                exit(EXIT_FAILURE);
            }
            productTemplate.push_back(std::tuple<int, SpeciesType>(SpeciesNamesDB::Instance()->stringToInt(name), SpeciesType::BULK));
        }
        
        else if(product.find("DIFFUSING") != std::string::npos) {
            
            ///Look up species, make sure in list
            std::string name = product.substr(0, product.find(":"));
            auto it = std::find_if(chemSR.speciesDiffusing.begin(), chemSR.speciesDiffusing.end(),
                                   [name](std::tuple<std::string, int, double> element) { return std::get<0>(element) == name ? true : false; });
            if(it == chemSR.speciesDiffusing.end()) {
                std::cout << "A diffusing species that was included in a reaction was not initialized. Exiting." << std::endl;
                exit(EXIT_FAILURE);
            }
            productTemplate.push_back(std::tuple<int, SpeciesType>(SpeciesNamesDB::Instance()->stringToInt(name), SpeciesType::DIFFUSING));
        }
        else {
            std::cout << "Second species listed in an unbinding reaction must be either bulk or diffusing. Exiting." << std::endl;
            exit(EXIT_FAILURE);
        }
        
        
        ///SECOND PRODUCT SPECIES MUST BE BOUND
        product = products[1];
        ///read strings, and look up type
        if(product.find("BOUND") != std::string::npos) {
            
            ///look up species, make sure in list
            std::string name = product.substr(0, product.find(":"));
            auto it = std::find(_speciesBound.begin(), _speciesBound.end(), name);
            int position = 0;
            
            if(it != _speciesBound.end()) {
                
                ///get position of iterator
                position = std::distance(_speciesBound.begin(), it);
                productTemplate.push_back(std::tuple<int, SpeciesType>(position, SpeciesType::BOUND));
            }
            else {
                std::cout << "A bound species that was included in a reaction was not initialized. Exiting." << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        else{
            std::cout << "Third species listed in an unbinding reaction must be bound. Exiting." << std::endl;
            exit(EXIT_FAILURE);
        }
        
        
        ///add reaction
        _filamentReactionTemplates.emplace_back(new UnbindingTemplate(reactantTemplate, productTemplate, std::get<2>(r)));
    }
    
    
    for(auto &r: chemSR.motorWalkingReactions) {
        
        std::vector<std::tuple<int, SpeciesType>> reactantTemplate;
        std::vector<std::tuple<int, SpeciesType>> productTemplate;
        
        std::vector<std::string> reactants = std::get<0>(r);
        std::vector<std::string> products = std::get<1>(r);
        ///read strings, and look up type
        ReactionType type;
        std::string species1;
        
        ///Checks on number of reactants, products
        if(reactants.size() != 2 || products.size() != 2) {
            std::cout << "Invalid motor walking reaction. Exiting." << std::endl;
            exit(EXIT_FAILURE);
        }
        
        ///FIRST REACTANT SPECIES MUST BE MOTOR
        auto reactant = reactants[0];
        if(reactant.find("MOTOR") != std::string::npos) {
            
            ///look up species, make sure in list
            std::string name = reactant.substr(0, reactant.find(":"));
            auto it = std::find(_speciesMotor.begin(), _speciesMotor.end(), name);
            int position = 0;
            
            if(it != _speciesMotor.end()) {
                
                species1 = name;
                
                if(reactant.find("N+1") != std::string::npos)
                    type = ReactionType::MOTORWALKINGBACKWARD;
                else type = ReactionType::MOTORWALKINGFORWARD;
                
                ///get position of iterator
                position = std::distance(_speciesMotor.begin(), it);
                reactantTemplate.push_back(std::tuple<int, SpeciesType>(position, SpeciesType::MOTOR));
            }
            else {
                std::cout << "A motor species that was included in a reaction was not initialized. Exiting." << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        else{
            std::cout << "First species listed in a motor walking reaction must be motor. Exiting." << std::endl;
            exit(EXIT_FAILURE);
        }

        
        ///SECOND REACTANT SPECIES MUST BE BOUND
        reactant = reactants[1];
        ///read strings, and look up type
        if(reactant.find("BOUND") != std::string::npos) {
            
            ///look up species, make sure in list
            std::string name = reactant.substr(0, reactant.find(":"));
            auto it = std::find(_speciesBound.begin(), _speciesBound.end(), name);
            int position = 0;
            
            if(it != _speciesBound.end()) {
                
                ///get position of iterator
                position = std::distance(_speciesBound.begin(), it);
                reactantTemplate.push_back(std::tuple<int, SpeciesType>(position, SpeciesType::BOUND));
            }
            else {
                std::cout << "A bound species that was included in a reaction was not initialized. Exiting." << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        else{
            std::cout << "Second species listed in a motor walking reaction must be bound. Exiting." << std::endl;
            exit(EXIT_FAILURE);
        }
   
        ///FIRST PRODUCT SPECIES MUST BE MOTOR
        auto product = products[0];
        if(product.find("MOTOR") != std::string::npos) {
            
            ///look up species, make sure in list
            std::string name = product.substr(0, product.find(":"));
            auto it = std::find(_speciesMotor.begin(), _speciesMotor.end(), name);
            int position = 0;
            
            if(name != species1) {
                std::cout << "Motor species in reactants and products of motor walking reaction must be same. Exiting." << std::endl;
                exit(EXIT_FAILURE);
            }
            
            if(it != _speciesMotor.end()) {
                
                ///get position of iterator
                position = std::distance(_speciesMotor.begin(), it);
                productTemplate.push_back(std::tuple<int, SpeciesType>(position, SpeciesType::MOTOR));
            }
            else {
                std::cout << "A motor species that was included in a reaction was not initialized. Exiting." << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        else {
            std::cout << "Third species listed in a motor walking reaction must be motor. Exiting." << std::endl;
            exit(EXIT_FAILURE);
        }
        
        ///SECOND PRODUCT SPECIES MUST BE BOUND
        ///read strings, and look up type
        product = products[1];
        if(product.find("BOUND") != std::string::npos) {
            
            ///look up species, make sure in list
            std::string name = product.substr(0, product.find(":"));
            auto it = std::find(_speciesBound.begin(), _speciesBound.end(), name);
            int position = 0;
            
            if(it != _speciesBound.end()) {
                
                ///get position of iterator
                position = std::distance(_speciesBound.begin(), it);
                productTemplate.push_back(std::tuple<int, SpeciesType>(position, SpeciesType::BOUND));
            }
            else {
                std::cout << "A bound species that was included in a reaction was not initialized. Exiting." << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        else{
            std::cout << "Fourth species listed in a motor walking reaction must be bound. Exiting." << std::endl;
            exit(EXIT_FAILURE);
        }

        ///add reaction
        if(type == ReactionType::MOTORWALKINGFORWARD)
            _filamentReactionTemplates.emplace_back(new MotorWalkingForwardTemplate(reactantTemplate, productTemplate, std::get<2>(r)));
        else
            _filamentReactionTemplates.emplace_back(new MotorWalkingBackwardTemplate(reactantTemplate, productTemplate, std::get<2>(r)));
    }
 
}

void InitializerImpl::generateCrossFilamentReactionTemplates(ChemistrySpeciesAndReactions& chemSR) {
    
    int ID = 0;
    for(auto &r: chemSR.linkerBindingReactions) {
        
        std::vector<std::tuple<int, SpeciesType>> reactantTemplate;
        std::vector<std::tuple<int, SpeciesType>> productTemplate;
        ReactionType type;
        std::string species1;
        
        std::vector<std::string> reactants = std::get<0>(r);
        std::vector<std::string> products = std::get<1>(r);
    
        ///Checks on number of reactants, products
        if(reactants.size() != 3 || products.size() != 2) {
            std::cout << "Invalid linker binding reaction. Exiting." << std::endl;
            exit(EXIT_FAILURE);
        }
        
        ///FIRST TWO SPECIES SHOULD BE BOUND
        auto reactant = reactants[0];
        if(reactant.find("BOUND") != std::string::npos) {
            
            ///look up species, make sure in list
            std::string name = reactant.substr(0, reactant.find(":"));
            auto it = std::find(_speciesBound.begin(), _speciesBound.end(), name);
            int position = 0;
            
            if(it != _speciesBound.end()) {
                
                ///get position of iterator
                position = std::distance(_speciesBound.begin(), it);
                reactantTemplate.push_back(std::tuple<int, SpeciesType>(position, SpeciesType::BOUND));
            }
            else {
                std::cout << "A bound species that was included in a reaction was not initialized. Exiting." << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        else {
            std::cout << "First species listed in a linker binding reaction must be bound. Exiting." << std::endl;
            exit(EXIT_FAILURE);
        }
        reactant = reactants[1];
        if(reactant.find("BOUND") != std::string::npos) {
            
            ///look up species, make sure in list
            std::string name = reactant.substr(0, reactant.find(":"));
            auto it = std::find(_speciesBound.begin(), _speciesBound.end(), name);
            int position = 0;
            
            if(it != _speciesBound.end()) {
                
                ///get position of iterator
                position = std::distance(_speciesBound.begin(), it);
                reactantTemplate.push_back(std::tuple<int, SpeciesType>(position, SpeciesType::BOUND));
            }
            else {
                std::cout << "A bound species that was included in a reaction was not initialized. Exiting." << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        else {
            std::cout << "Second species listed in a linker binding reaction must be bound. Exiting." << std::endl;
            exit(EXIT_FAILURE);
        }
        
        ///THIRD REACTANT SPECIES SHOULD BE BULK OR DIFFUSING
        reactant = reactants[2];
        if(reactant.find("BULK") != std::string::npos) {
            
            ///Look up species, make sure in list
            std::string name = reactant.substr(0, reactant.find(":"));
            auto it = std::find_if(chemSR.speciesBulk.begin(), chemSR.speciesBulk.end(),
                                   [name](std::tuple<std::string, int> element) { return std::get<0>(element) == name ? true : false; });
            
            if(it == chemSR.speciesBulk.end()) {
                std::cout << "A bulk species that was included in a reaction was not initialized. Exiting." << std::endl;
                exit(EXIT_FAILURE);
            }
            reactantTemplate.push_back(std::tuple<int, SpeciesType>(SpeciesNamesDB::Instance()->stringToInt(name), SpeciesType::BULK));
        }
        
        else if(reactant.find("DIFFUSING") != std::string::npos) {
            
            ///Look up species, make sure in list
            std::string name = reactant.substr(0, reactant.find(":"));
            auto it = std::find_if(chemSR.speciesDiffusing.begin(), chemSR.speciesDiffusing.end(),
                                   [name](std::tuple<std::string, int, double> element) { return std::get<0>(element) == name ? true : false; });
            if(it == chemSR.speciesDiffusing.end()) {
                std::cout << "A diffusing species that was included in a reaction was not initialized. Exiting." << std::endl;
                exit(EXIT_FAILURE);
            }
            reactantTemplate.push_back(std::tuple<int, SpeciesType>(SpeciesNamesDB::Instance()->stringToInt(name), SpeciesType::DIFFUSING));
        }
        else {
            std::cout << "Third species listed in a linker binding reaction must be bulk or diffusing. Exiting." << std::endl;
            exit(EXIT_FAILURE);
        }
        

        ///FIRST TWO SPECIES IN PRODUCTS MUST BE LINKER
        auto product = products[0];
        
        if(product.find("LINKER") != std::string::npos) {
            
            type = ReactionType::LINKERBINDING;
            
            ///look up species, make sure in list
            std::string name = product.substr(0, product.find(":"));
            auto it = std::find(_speciesLinker.begin(), _speciesLinker.end(), name);
            int position = 0;
            
            species1 = name;
            
            if(it != _speciesLinker.end()) {
                
                ///get position of iterator
                position = std::distance(_speciesLinker.begin(), it);
                productTemplate.push_back(std::tuple<int, SpeciesType>(position, SpeciesType::LINKER));
            }
            else {
                std::cout << "A linker species that was included in a reaction was not initialized. Exiting." << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        else {
            std::cout << "Fourth species listed in a linker binding reaction must be linker. Exiting." << std::endl;
            exit(EXIT_FAILURE);
        }
        
        product = products[1];
        if(product.find("LINKER") != std::string::npos) {
            
            ///look up species, make sure in list
            std::string name = product.substr(0, product.find(":"));
            auto it = std::find(_speciesLinker.begin(), _speciesLinker.end(), name);
            int position = 0;
            
            if(name != species1) {
                std::cout << "Linker species in reactants and products of linker binding reaction must be same. Exiting." << std::endl;
                exit(EXIT_FAILURE);
            }
        
            if(it != _speciesLinker.end()) {
                
                ///get position of iterator
                position = std::distance(_speciesLinker.begin(), it);
                productTemplate.push_back(std::tuple<int, SpeciesType>(position, SpeciesType::LINKER));
            }
            else {
                std::cout << "A linker species that was included in a reaction was not initialized. Exiting." << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        
        else {
            std::cout << "Fifth species listed in a linker binding reaction must be linker. Exiting." << std::endl;
            exit(EXIT_FAILURE);
        }
    
        _crossFilamentReactionTemplates.emplace_back(
            new LinkerBindingTemplate(reactantTemplate, productTemplate, std::get<2>(r), std::get<3>(r), std::get<4>(r), ID++));
    }
    
    for(auto &r: chemSR.motorBindingReactions) {
        
        std::vector<std::tuple<int, SpeciesType>> reactantTemplate;
        std::vector<std::tuple<int, SpeciesType>> productTemplate;
        ReactionType type;
        std::string species1;
        
        std::vector<std::string> reactants = std::get<0>(r);
        std::vector<std::string> products = std::get<1>(r);
        
        ///Checks on number of reactants, products
        if(reactants.size() != 3 || products.size() != 2) {
            std::cout << "Invalid motor binding reaction. Exiting." << std::endl;
            exit(EXIT_FAILURE);
        }
        
        ///FIRST TWO SPECIES SHOULD BE BOUND
        auto reactant = reactants[0];
        if(reactant.find("BOUND") != std::string::npos) {
            
            ///look up species, make sure in list
            std::string name = reactant.substr(0, reactant.find(":"));
            auto it = std::find(_speciesBound.begin(), _speciesBound.end(), name);
            int position = 0;
            
            if(it != _speciesBound.end()) {
                
                ///get position of iterator
                position = std::distance(_speciesBound.begin(), it);
                reactantTemplate.push_back(std::tuple<int, SpeciesType>(position, SpeciesType::BOUND));
            }
            else {
                std::cout << "A bound species that was included in a reaction was not initialized. Exiting." << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        else {
            std::cout << "First species listed in a motor binding reaction must be bound. Exiting." << std::endl;
            exit(EXIT_FAILURE);
        }
        reactant = reactants[1];
        if(reactant.find("BOUND") != std::string::npos) {
            
            ///look up species, make sure in list
            std::string name = reactant.substr(0, reactant.find(":"));
            auto it = std::find(_speciesBound.begin(), _speciesBound.end(), name);
            int position = 0;
            
            if(it != _speciesBound.end()) {
                
                ///get position of iterator
                position = std::distance(_speciesBound.begin(), it);
                reactantTemplate.push_back(std::tuple<int, SpeciesType>(position, SpeciesType::BOUND));
            }
            else {
                std::cout << "A bound species that was included in a reaction was not initialized. Exiting." << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        else {
            std::cout << "Second species listed in a motor binding reaction must be bound. Exiting." << std::endl;
            exit(EXIT_FAILURE);
        }
        
        ///THIRD REACTANT SPECIES SHOULD BE BULK OR DIFFUSING
        reactant = reactants[2];
        if(reactant.find("BULK") != std::string::npos) {
            
            ///Look up species, make sure in list
            std::string name = reactant.substr(0, reactant.find(":"));
            auto it = std::find_if(chemSR.speciesBulk.begin(), chemSR.speciesBulk.end(),
                                   [name](std::tuple<std::string, int> element) { return std::get<0>(element) == name ? true : false; });
            
            if(it == chemSR.speciesBulk.end()) {
                std::cout << "A bulk species that was included in a reaction was not initialized. Exiting." << std::endl;
                exit(EXIT_FAILURE);
            }
            reactantTemplate.push_back(std::tuple<int, SpeciesType>(SpeciesNamesDB::Instance()->stringToInt(name), SpeciesType::BULK));
        }
        
        else if(reactant.find("DIFFUSING") != std::string::npos) {
            
            ///Look up species, make sure in list
            std::string name = reactant.substr(0, reactant.find(":"));
            auto it = std::find_if(chemSR.speciesDiffusing.begin(), chemSR.speciesDiffusing.end(),
                                   [name](std::tuple<std::string, int, double> element) { return std::get<0>(element) == name ? true : false; });
            if(it == chemSR.speciesDiffusing.end()) {
                std::cout << "A diffusing species that was included in a reaction was not initialized. Exiting." << std::endl;
                exit(EXIT_FAILURE);
            }
            reactantTemplate.push_back(std::tuple<int, SpeciesType>(SpeciesNamesDB::Instance()->stringToInt(name), SpeciesType::DIFFUSING));
        }
        else {
            std::cout << "Third species listed in a cross filament binding reaction must be bulk or diffusing. Exiting." << std::endl;
            exit(EXIT_FAILURE);
        }
        
        
        ///FIRST TWO SPECIES IN PRODUCTS MUST BE MOTOR
        auto product = products[0];
        
        if(product.find("MOTOR") != std::string::npos) {
            
            type = ReactionType::MOTORBINDING;
            
            ///look up species, make sure in list
            std::string name = product.substr(0, product.find(":"));
            auto it = std::find(_speciesMotor.begin(), _speciesMotor.end(), name);
            int position = 0;
            
            species1 = name;
            
            if(it != _speciesMotor.end()) {
                
                ///get position of iterator
                position = std::distance(_speciesMotor.begin(), it);
                productTemplate.push_back(std::tuple<int, SpeciesType>(position, SpeciesType::MOTOR));
            }
            else {
                std::cout << "A motor species that was included in a reaction was not initialized. Exiting." << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        else {
            std::cout << "Fourth species listed in a motor binding reaction must be motor. Exiting." << std::endl;
            exit(EXIT_FAILURE);
        }
        
        product = products[1];
        if(product.find("MOTOR") != std::string::npos) {
            
            ///look up species, make sure in list
            std::string name = product.substr(0, product.find(":"));
            auto it = std::find(_speciesMotor.begin(), _speciesMotor.end(), name);
            int position = 0;
            
            if(name != species1) {
                std::cout << "Motor species in reactants and products of motor binding reaction must be same. Exiting." << std::endl;
                exit(EXIT_FAILURE);
            }
 
            if(it != _speciesMotor.end()) {
                
                ///get position of iterator
                position = std::distance(_speciesMotor.begin(), it);
                productTemplate.push_back(std::tuple<int, SpeciesType>(position, SpeciesType::MOTOR));
            }
            else {
                std::cout << "A motor species that was included in a reaction was not initialized. Exiting." << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        
        else {
            std::cout << "Fifth species listed in a motor binding reaction must be motor. Exiting." << std::endl;
            exit(EXIT_FAILURE);
        }
        
        _crossFilamentReactionTemplates.emplace_back(
           new MotorBindingTemplate(reactantTemplate, productTemplate, std::get<2>(r), std::get<3>(r), std::get<4>(r), ID++));
    }
}


void InitializerImpl::generateGeneralReactions(ChemistrySpeciesAndReactions& chemSR, Compartment& protoCompartment) {
    
     ///go through reactions, add each
    for(auto &r: chemSR.genReactions) {
    
        std::vector<Species*> reactantSpecies;
        std::vector<Species*> productSpecies;
        
        std::vector<std::string> reactants = std::get<0>(r);
        std::vector<std::string> products = std::get<1>(r);
        
        for(auto &reactant : reactants) {
            if(reactant.find("BULK") != std::string::npos) {
                
                ///Look up species, make sure in list
                std::string name = reactant.substr(0, reactant.find(":"));
                auto it = std::find_if(chemSR.speciesBulk.begin(), chemSR.speciesBulk.end(),
                                       [name](std::tuple<std::string, int> element) { return std::get<0>(element) == name ? true : false; });
                
                if(it == chemSR.speciesBulk.end()) {
                    std::cout << "A bulk species that was included in a reaction was not initialized. Exiting." << std::endl;
                    exit(EXIT_FAILURE);
                }
                reactantSpecies.push_back(CompartmentGrid::Instance(compartmentGridKey())->findSpeciesBulkByName(name));
            }
            
            else if(reactant.find("DIFFUSING") != std::string::npos) {
                
                ///Look up species, make sure in list
                std::string name = reactant.substr(0, reactant.find(":"));
                auto it = std::find_if(chemSR.speciesDiffusing.begin(), chemSR.speciesDiffusing.end(),
                                       [name](std::tuple<std::string, int, double> element) { return std::get<0>(element) == name ? true : false; });
                if(it == chemSR.speciesDiffusing.end()) {
                    std::cout << "A diffusing species that was included in a reaction was not initialized. Exiting." << std::endl;
                    exit(EXIT_FAILURE);
                }
                reactantSpecies.push_back(protoCompartment.findSpeciesByName(name));
            }
            else {
                std::cout << "All reactants and products in a general reaction must be bulk or diffusing. Exiting." << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        
        for(auto &product : products) {
            if(product.find("BULK") != std::string::npos) {
                
                ///Look up species, make sure in list
                std::string name = product.substr(0, product.find(":"));
                auto it = std::find_if(chemSR.speciesBulk.begin(), chemSR.speciesBulk.end(),
                                       [name](std::tuple<std::string, int> element) { return std::get<0>(element) == name ? true : false; });
                
                if(it == chemSR.speciesBulk.end()) {
                    std::cout << "A bulk species that was included in a reaction was not initialized. Exiting." << std::endl;
                    exit(EXIT_FAILURE);
                }
                productSpecies.push_back(CompartmentGrid::Instance(compartmentGridKey())->findSpeciesBulkByName(name));
            }
            
            else if(product.find("DIFFUSING") != std::string::npos) {
                
                ///Look up species, make sure in list
                std::string name = product.substr(0, product.find(":"));
                auto it = std::find_if(chemSR.speciesDiffusing.begin(), chemSR.speciesDiffusing.end(),
                                       [name](std::tuple<std::string, int, double> element) { return std::get<0>(element) == name ? true : false; });
                if(it == chemSR.speciesDiffusing.end()) {
                    std::cout << "A diffusing species that was included in a reaction was not initialized. Exiting." << std::endl;
                    exit(EXIT_FAILURE);
                }
                productSpecies.push_back(protoCompartment.findSpeciesByName(name));
            }
            else {
                std::cout << "All reactants and products in a general reaction must be bulk or diffusing. Exiting." << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        ///add the reaction
        std::vector<Species*> species = reactantSpecies;
        species.insert(species.end(), productSpecies.begin(), productSpecies.end());
        
        
        ReactionBase* rxn;
        ///create the reaction
        
        ///<1,1>
        if(reactantSpecies.size() == 1 && productSpecies.size() == 1)
            rxn = new Reaction<1,1>(species, std::get<2>(r), true);
        ///<2,1>
        else if(reactantSpecies.size() == 2 && productSpecies.size() == 1)
            rxn = new Reaction<2,1>(species, std::get<2>(r), true);
        ///<1,2>
        else if(reactantSpecies.size() == 1 && productSpecies.size() == 2)
            rxn = new Reaction<1,2>(species, std::get<2>(r), true);
        ///<2,0>
        else if(reactantSpecies.size() == 2 && productSpecies.size() == 0)
            rxn = new Reaction<2,0>(species, std::get<2>(r), true);
        ///<2,2>
        else if(reactantSpecies.size() == 2 && productSpecies.size() == 2)
            rxn = new Reaction<2,2>(species, std::get<2>(r), true);
        ///<1,3>
        else if(reactantSpecies.size() == 1 && productSpecies.size() == 3)
            rxn = new Reaction<1,3>(species, std::get<2>(r), true);
        ///<2,2>
        else if(reactantSpecies.size() == 2 && productSpecies.size() == 3)
            rxn = new Reaction<2,3>(species, std::get<2>(r), true);
        ///<3,2>
        else if(reactantSpecies.size() == 3 && productSpecies.size() == 2)
            rxn = new Reaction<3,2>(species, std::get<2>(r), true);
        else {
            std::cout << "General reaction specified does not match any existing templates. Exiting." <<std::endl;
            exit(EXIT_FAILURE);
        }
        
        ///add to compartment
        protoCompartment.addInternalReactionUnique(std::unique_ptr<ReactionBase>(rxn));
    }
}

void InitializerImpl::generateBulkReactions(ChemistrySpeciesAndReactions& chemSR) {
    
    ///go through reactions, add each
    for(auto &r: chemSR.bulkReactions) {
        
        std::vector<Species*> reactantSpecies;
        std::vector<Species*> productSpecies;
        
        std::vector<std::string> reactants = std::get<0>(r);
        std::vector<std::string> products = std::get<1>(r);
        
        for(auto &reactant : reactants) {
            if(reactant.find("BULK") != std::string::npos) {
                
                ///Look up species, make sure in list
                std::string name = reactant.substr(0, reactant.find(":"));
                auto it = std::find_if(chemSR.speciesBulk.begin(), chemSR.speciesBulk.end(),
                                       [name](std::tuple<std::string, int> element) { return std::get<0>(element) == name ? true : false; });
                
                if(it == chemSR.speciesBulk.end()) {
                    std::cout << "A bulk species that was included in a reaction was not initialized. Exiting." << std::endl;
                    exit(EXIT_FAILURE);
                }
                reactantSpecies.push_back(CompartmentGrid::Instance(compartmentGridKey())->findSpeciesBulkByName(name));
            }
            else {
                std::cout << "All reactants and products in a bulk reaction must be bulk. Exiting." << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        
        for(auto &product : products) {
            if(product.find("BULK") != std::string::npos) {
                
                ///Look up species, make sure in list
                std::string name = product.substr(0, product.find(":"));
                auto it = std::find_if(chemSR.speciesBulk.begin(), chemSR.speciesBulk.end(),
                                       [name](std::tuple<std::string, int> element) { return std::get<0>(element) == name ? true : false; });
                
                if(it == chemSR.speciesBulk.end()) {
                    std::cout << "A bulk species that was included in a reaction was not initialized. Exiting." << std::endl;
                    exit(EXIT_FAILURE);
                }
                productSpecies.push_back(CompartmentGrid::Instance(compartmentGridKey())->findSpeciesBulkByName(name));
            }
            else {
                std::cout << "All reactants and products in a bulk reaction must be bulk. Exiting." << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        ///add the reaction
        std::vector<Species*> species = reactantSpecies;
        species.insert(species.end(), productSpecies.begin(), productSpecies.end());
        
        ReactionBase* rxn;
        ///create the reaction
        
        ///<1,1>
        if(reactantSpecies.size() == 1 && productSpecies.size() == 1)
            rxn = new Reaction<1,1>(species, std::get<2>(r));
        ///<2,1>
        else if(reactantSpecies.size() == 2 && productSpecies.size() == 1)
            rxn = new Reaction<2,1>(species, std::get<2>(r));
        ///<1,2>
        else if(reactantSpecies.size() == 1 && productSpecies.size() == 2)
            rxn = new Reaction<1,2>(species, std::get<2>(r));
        ///<2,0>
        else if(reactantSpecies.size() == 2 && productSpecies.size() == 0)
            rxn = new Reaction<2,0>(species, std::get<2>(r));
        ///<2,2>
        else if(reactantSpecies.size() == 2 && productSpecies.size() == 2)
            rxn = new Reaction<2,2>(species, std::get<2>(r));
        ///<1,3>
        else if(reactantSpecies.size() == 1 && productSpecies.size() == 3)
            rxn = new Reaction<1,3>(species, std::get<2>(r));
        ///<2,2>
        else if(reactantSpecies.size() == 2 && productSpecies.size() == 3)
            rxn = new Reaction<2,3>(species, std::get<2>(r));
        ///<3,2>
        else if(reactantSpecies.size() == 3 && productSpecies.size() == 2)
            rxn = new Reaction<3,2>(species, std::get<2>(r));
        else {
            std::cout << "Bulk reaction specified does not match any existing templates. Exiting." <<std::endl;
            exit(EXIT_FAILURE);
        }
        
        ///add to grid
        CompartmentGrid::Instance(compartmentGridKey())->addBulkReactionUnique(std::unique_ptr<ReactionBase>(rxn));
    }
}



void InitializerImpl::initialize(ChemistrySpeciesAndReactions& chemSR) {
    
    ///set static system ptr
    ReactionFilamentTemplate::_ps = _subSystem;
    ReactionCrossFilamentTemplate::_ps = _subSystem;
    
    ///Copy all species from chemSR struct
    _speciesFilament =  chemSR.speciesFilament;
    _speciesPlusEnd  =  chemSR.speciesPlusEnd;
    _speciesMinusEnd =  chemSR.speciesMinusEnd;
    
    _speciesBound  =  chemSR.speciesBound;
    _speciesLinker =  chemSR.speciesLinker;
    _speciesMotor  =  chemSR.speciesMotor;
    
    ///Setup all species diffusing and bulk
    Compartment& cProto = CompartmentGrid::Instance(compartmentGridKey())->getProtoCompartment();
    
    for(auto &sd : chemSR.speciesDiffusing)
        cProto.addSpeciesUnique(std::unique_ptr<Species>(
           new SpeciesDiffusing(std::get<0>(sd), std::get<1>(sd))), std::get<2>(sd));
    
    for(auto &sb : chemSR.speciesBulk)
        CompartmentGrid::Instance(compartmentGridKey())->addSpeciesBulk(std::get<0>(sb), std::get<1>(sb));
    
    ///add reactions to protocompartment
    generateGeneralReactions(chemSR, cProto);
    ///generate any bulk reactions
    generateBulkReactions(chemSR);
    
    ///initialize all compartments equivalent to cproto
    for(auto &c : CompartmentGrid::Instance(compartmentGridKey())->children()) {
        Compartment *C = static_cast<Compartment*>(c.get());
        *C = cProto;
    }
    for(auto &c : CompartmentGrid::Instance(compartmentGridKey())->children()) {
        Compartment *C = static_cast<Compartment*>(c.get());
        C->generateAllDiffusionReactions();
    }
    
    ///add reactions to chemsim
    CompartmentGrid::Instance(compartmentGridKey())->addChemSimReactions();
    
    ///create filament reaction templates
    generateFilamentReactionTemplates(chemSR);
    ///create cross filament reaction templates
    generateCrossFilamentReactionTemplates(chemSR);
}

CCylinder* InitializerImpl::createCCylinder(Filament *pf, Compartment* c,
                                            bool extensionFront, bool extensionBack, bool creation)
{
    CCylinder* cc = new CCylinder(c);
    
    ///maxlength is same length as mcylinder
    int maxlength = cc->size();
    
    ///add monomers to cylinder
    for(int i = 0; i < maxlength; i++) {
        
        CMonomer* m = new CMonomer();
        for(auto &f : _speciesFilament) {
            SpeciesFilament* sf =
                c->addSpeciesFilament(
                SpeciesNamesDB::Instance()->generateUniqueName(f));
            m->addSpeciesFilament(sf);
        }
        for (auto &p : _speciesPlusEnd) {
            SpeciesPlusEnd* sp =
                c->addSpeciesPlusEnd(
                SpeciesNamesDB::Instance()->generateUniqueName(p));
            m->addSpeciesPlusEnd(sp);
        }
        for (auto &mi : _speciesMinusEnd) {
            SpeciesMinusEnd* smi =
                c->addSpeciesMinusEnd(
                SpeciesNamesDB::Instance()->generateUniqueName(mi));
            m->addSpeciesMinusEnd(smi);
        }
        
        for (auto &b : _speciesBound) {
            SpeciesBound* sb =
                c->addSpeciesBound(
                SpeciesNamesDB::Instance()->generateUniqueName(b));
            m->addSpeciesBound(sb);
        }
        for (auto &l : _speciesLinker) {
            SpeciesLinker* sl =
                c->addSpeciesLinker(
                SpeciesNamesDB::Instance()->generateUniqueName(l));
            m->addSpeciesLinker(sl);
        }
        for (auto &mo : _speciesMotor) {
            SpeciesMotor* sm =
                c->addSpeciesMotor(
                SpeciesNamesDB::Instance()->generateUniqueName(mo));
            m->addSpeciesMotor(sm);
        }
        
        cc->addCMonomer(m);
    }
    
    ///Add all reaction templates to this cylinder
    for(auto &r : _filamentReactionTemplates) { r->addReaction(cc, pf); }
    
    ///get last ccylinder
    CCylinder* lastcc = nullptr;
 
    ///extension of front
    if(extensionFront) {
        lastcc = pf->getCylinderVector().back()->getCCylinder();
        for(auto &r : _filamentReactionTemplates) r->addReaction(lastcc, cc, pf);
    }
    ///extension of back
    else if(extensionBack) {
        lastcc = pf->getCylinderVector().front()->getCCylinder();
        for(auto &r : _filamentReactionTemplates) r->addReaction(cc, lastcc, pf);
    }

    else if(creation) {
        
        CMonomer* m2 = lastcc->getCMonomer(int(cc->size() / 2));
        CMonomer* m1 = lastcc->getCMonomer(int(cc->size() / 2) - 1);
        m2->speciesPlusEnd(0)->getRSpecies().setN(1);
        m1->speciesMinusEnd(0)->getRSpecies().setN(1);
    }
    
    ///Base case, initialization
    else {
        ///Check if this is the first cylinder
        if(pf->getCylinderVector().size() != 0) {
            
            ///remove plus end from last, add to this.
            lastcc = pf->getCylinderVector().back()->getCCylinder();
            CMonomer* m1 = lastcc->getCMonomer(lastcc->size() - 2);
            m1->speciesPlusEnd(0)->getRSpecies().setN(0);
            
            CMonomer* m2 = cc->getCMonomer(cc->size() - 2);
            m2->speciesPlusEnd(0)->getRSpecies().setN(1);
            m2->speciesBound(0)->getRSpecies().setN(1);
            
            ///fill last cylinder with default filament value
            for(int i = lastcc->size() - 2; i < lastcc->size(); i++) {
                lastcc->getCMonomer(i)->speciesFilament(0)->getRSpecies().setN(1);
                lastcc->getCMonomer(i)->speciesBound(0)->getRSpecies().setN(1);
                
            }
            ///fill new cylinder with default filament value
            for(int i = 0; i < cc->size() - 2; i++) {
                cc->getCMonomer(i)->speciesFilament(0)->getRSpecies().setN(1);
                cc->getCMonomer(i)->speciesBound(0)->getRSpecies().setN(1);
            }
            
            for(auto &r : _filamentReactionTemplates) r->addReaction(lastcc, cc, pf);
            
        }
        ///this is first one
        else {
            //set back and front
            CMonomer* m1 = cc->getCMonomer(cc->size() - 2);
            m1->speciesPlusEnd(0)->getRSpecies().setN(1);
            m1->speciesBound(0)->getRSpecies().setN(1);
            
            CMonomer* m2 = cc->getCMonomer(1);
            m2->speciesMinusEnd(0)->getRSpecies().setN(1);
            m2->speciesBound(0)->getRSpecies().setN(1);
            
            ///fill with default filament value
            for(int i = 2; i < cc->size() - 2; i++) {
                cc->getCMonomer(i)->speciesFilament(0)->getRSpecies().setN(1);
                cc->getCMonomer(i)->speciesBound(0)->getRSpecies().setN(1);
            }
        }
    }
    
    //update all reactions added if not initialization
    if(creation || extensionFront || extensionBack) cc->updateReactions();
    
    ///clean up and return
    return cc;
}

void InitializerImpl::updateCCylinder(CCylinder* cc, std::vector<CCylinder*>& cNeighbors) {
    
    ///loop through the cylinders reaction map, and remove any that are out of range.
    auto ccReactions = cc->getCrossCylinderReactions();
    for(auto it = ccReactions.begin(); it != ccReactions.end(); it++) {
        
        ///calculate distance from this CCylinder to the one in the map
        double dist = TwoPointDistance(cc->getCylinder()->coordinate, it->first->getCylinder()->coordinate);
        
        for(auto &r : it->second) {
            ///if out of range, remove it
            if((r->getRMin() > dist || r->getRMax() < dist) && (r->getRMin() != 0.0 && r->getRMax() != 0.0))
                cc->removeCrossCylinderReaction(it->first, r);
        }
        
        ///If the number of reactions between this cylinder and the other has dropped to
        ///zero, remove the key/value as well as this reacting cylinder in other
        if(it->second.size() == 0)
            it->first->removeReactingCylinder(cc);
    }
    
    ///Now, add new cross filament reactions
    for(auto &rTemplate : _crossFilamentReactionTemplates) {
    
        /// loop through all cylinders in range
        for(auto &ccNeighbor : cNeighbors) {
            double dist = TwoPointDistance(cc->getCylinder()->coordinate, ccNeighbor->getCylinder()->coordinate);
                
            ///Check if this cross filament reaction is already there. If not, add it
            if(cc->getCylinder()->getFilament() != ccNeighbor->getCylinder()->getFilament()
               && (rTemplate->getRMin() < dist && rTemplate->getRMax() > dist)) {
                
                bool hasRxn = false;
                for(auto &r : cc->getCrossCylinderReactions()[ccNeighbor])
                    if(r->getReactionID() == rTemplate->getReactionID()) { hasRxn = true; break;}

                if(!hasRxn) rTemplate->addReaction(cc, ccNeighbor);
            }
        }
    }
    cc->updateReactions();
}


