
//------------------------------------------------------------------
//  **MEDYAN** - Simulation Package for the Mechanochemical
//               Dynamics of Active Networks, v4.0
//
//  Copyright (2015-2018)  Papoian Lab, University of Maryland
//
//                 ALL RIGHTS RESERVED
//
//  See the MEDYAN web page for more information:
//  http://www.medyan.org
//------------------------------------------------------------------

#ifndef MEDYAN_Parser_h
#define MEDYAN_Parser_h

#include <algorithm>
#include <cctype> // isspace
#include <filesystem>
#include <fstream>
#include <functional>
#include <iterator>
#include <list>
#include <map>
#include <stdexcept>
#include <string>
#include <sstream>
#include <type_traits>
#include <variant>
#include <vector>

#include "common.h"
#include "SysParams.h"
#include "utility.h"

namespace medyan {

// A tokenizer for input files
//
// This extends the input config to s-expressions, which allows more
// extensibility, but it does not include the lisp syntax
//
// The input file will be treated as a list of s-expressions. For backwards
// compatibility and simplicity, several special specifications exist:
//   - All symbols are parsed as strings.
//   - '#' and ';' both start a line comment.
//   - If a top level (not within any parentheses) token is not a parenthesis,
//     an implicit pair of parentheses will be added before the token and
//     before the next closest top level line break or end of input.
//   - Double quotation mark is parsed as string, but nested quotations,
//     escapings are not allowed. Line comment marker in a quoted string stay
//     has no effect.
//   - cons that is not a list is currently not supported.
//   - Evaluation is currently not supported.
//   - Most syntactic sugar is currently not supported.

struct ConfigFileToken {
    enum class Type {
        string,
        comment,
        parenthesisLeft,
        parenthesisRight,
        lineBreak,         // needed for implicit parenthesis
        unknown
    };

    Type        type = Type::string;
    std::string content;

    static std::string defaultContent(Type type) {
        switch(type) {
        case Type::string:           return "";
        case Type::comment:          return "";
        case Type::parenthesisLeft:  return "(";
        case Type::parenthesisRight: return ")";
        case Type::lineBreak:        return "\n";
        case Type::unknown:          return "";
        default:                     return "";
        }
    }

    static auto makeDefault(Type type) {
        return ConfigFileToken { type, defaultContent(type) };
    }

    static auto makeString(std::string s) {
        return ConfigFileToken { Type::string, std::move(s) };
    }
};

struct SExpr {
    using StringType = std::string;
    using ListType   = std::vector< SExpr >;

    struct Car {
        SExpr operator()(const StringType&) const {
            LOG(ERROR) << "Expected cons in Car, but got a string.";
            throw std::runtime_error("Invalid argument in Car");
        }
        SExpr operator()(const ListType& l) const {
            return SExpr { l[0] };
        }
    };
    struct Cdr {
        SExpr operator()(const StringType&) const {
            LOG(ERROR) << "Expected cons in Cdr, but got a string.";
            throw std::runtime_error("Invalid argument in Cdr");
        }
        SExpr operator()(const ListType& l) const {
            if(l.empty()) {
                LOG(ERROR) << "Expected cons in Cdr, but got an empty list.";
                throw std::runtime_error("Invalid argument in Cdr");
            }
            else {
                return SExpr { ListType(l.begin() + 1, l.end()) };
            }
        }
    };

    std::variant<
        StringType,
        ListType
    > data;
};

inline SExpr car(const SExpr& se) {
    return std::visit(SExpr::Car{}, se.data);
}
inline SExpr cdr(const SExpr& se) {
    return std::visit(SExpr::Cdr{}, se.data);
}
inline std::vector< std::string > getStringVector(const SExpr::ListType& sel) {
    // Precondition:
    //   - Every element in sel is of string type
    using namespace std;

    vector< string > res;
    res.reserve(sel.size());
    for(const auto& eachSE : sel) {
        res.push_back(get< SExpr::StringType >(eachSE.data));
    }
    return res;
}


inline bool isTokenSpecialChar(char x) {
    return std::isspace(x) ||
        x == '#' || x == ';' ||
        x == '(' || x == ')' ||
        x == '"';
}

// Tokenize and the input file.
inline std::list< ConfigFileToken > tokenizeConfigFile(const std::string& str) {
    const auto len = str.length();

    std::list< ConfigFileToken > tokens;

    for(int i = 0; i < len; ++i) {
        const char a = str[i];
        if(a == '\n') {
            tokens.push_back(ConfigFileToken::makeDefault(ConfigFileToken::Type::lineBreak));
        }
        else if(std::isspace(a)) {
            // Do nothing
        }
        else if(a == '#' || a == ';') {
            int j = i + 1;
            while(j < len && str[j] != '\n') ++j;

            // Now j points to either the end of string, or the next line break
            tokens.push_back({ConfigFileToken::Type::comment, str.substr(i, j-i)});
            i = j - 1;
        }
        else if(a == '(') {
            tokens.push_back(ConfigFileToken::makeDefault(ConfigFileToken::Type::parenthesisLeft));
        }
        else if(a == ')') {
            tokens.push_back(ConfigFileToken::makeDefault(ConfigFileToken::Type::parenthesisRight));
        }
        else if(a == '"') {
            int j = i + 1;
            while(j < len && str[j] != '"') ++j;

            // Now j points to either end of string, or the next double quotation
            if(j < len) {
                tokens.push_back(ConfigFileToken::makeString(str.substr(i+1, j-i-1)));
                i = j;
            } else {
                LOG(ERROR) << "Quotation marks do not match";
                throw std::runtime_error("Quotation marks do not match.");
            }
        }
        else {
            int j = i + 1;
            while(j < len && !isTokenSpecialChar(str[j])) ++j;

            // Now j points to either the end of string, or the next special char
            tokens.push_back(ConfigFileToken::makeString(str.substr(i, j-i)));
            i = j - 1;
        }
    }

    return tokens;
}

inline void outputConfigTokens(std::ostream& os, const std::list< ConfigFileToken >& tokens) {
    auto lastType = ConfigFileToken::Type::unknown;
    for(const auto& tok : tokens) {
        if(tok.type == ConfigFileToken::Type::string) {

            auto content = tok.content;
            // Strip double quotations
            content.erase(
                std::remove(content.begin(), content.end(), '"'),
                content.end()
            );

            // Check special characters exist
            const bool hasSpace = (
                std::find_if(content.begin(), content.end(), isTokenSpecialChar)
                    != content.end());

            if(
                lastType == ConfigFileToken::Type::string ||
                lastType == ConfigFileToken::Type::parenthesisRight
            ) {
                os << ' ';
            }

            if(hasSpace || content.empty()) {
                os << '"' << content << '"';
            } else {
                os << content;
            }
        }
        else if(tok.type == ConfigFileToken::Type::comment) {
            if(
                lastType != ConfigFileToken::Type::lineBreak &&
                lastType != ConfigFileToken::Type::unknown
            ) {
                os << ' ';
            }
            os << tok.content;
        }
        else if(tok.type == ConfigFileToken::Type::parenthesisLeft) {
            if(
                lastType == ConfigFileToken::Type::string ||
                lastType == ConfigFileToken::Type::parenthesisRight
            ) {
                os << ' ';
            }

            os << tok.content;
        }
        else {
            os << tok.content;
        }

        lastType = tok.type;
    }
}

inline SExpr lexConfigTokensList(
    const std::list< ConfigFileToken >&           tokens,
    std::list< ConfigFileToken >::const_iterator& tokenIter,
    const int                                     depth,
    const bool                                    implicitParentheses
) {
    using namespace std;
    using VS = vector< SExpr >;

    SExpr se;
    se.data = VS{};

    while(tokenIter != tokens.cend()) {
        if(tokenIter->type == ConfigFileToken::Type::string) {
            // Add to the current list
            get<VS>(se.data).push_back(
                SExpr { tokenIter->content }
            );
            ++tokenIter;
        }
        else if(tokenIter->type == ConfigFileToken::Type::parenthesisLeft) {
            get<VS>(se.data).push_back(
                lexConfigTokensList(tokens, ++tokenIter, depth + 1, false)
            );
        }
        else if(tokenIter->type == ConfigFileToken::Type::parenthesisRight) {
            if(implicitParentheses) {
                LOG(ERROR) << "Unexpected ')' in implicit parentheses.";
                throw runtime_error("Unmatched parentheses");
            } else {
                // End current list
                ++tokenIter;
                return se;
            }
        }
        else if(tokenIter->type == ConfigFileToken::Type::lineBreak) {
            if(implicitParentheses) {
                // End current list
                ++tokenIter;
                return se;
            }
            else {
                ++tokenIter;
            }
        }
        else {
            ++tokenIter;
        }
    }

    if(implicitParentheses) {
        // Reaching the end
        return se;
    }
    else {
        LOG(ERROR) << "')' is expected, but not found.";
        throw runtime_error("Unmatched parentheses");
    }
}
inline SExpr lexConfigTokens(const list< ConfigFileToken >& tokens) {
    using namespace std;
    using VS = vector< SExpr >;

    SExpr se;
    se.data = VS{};

    auto tokenIter = tokens.cbegin();
    for(auto tokenIter = tokens.cbegin(); tokenIter != tokens.cend(); ) {

        if (tokenIter->type == ConfigFileToken::Type::string) {
            // Implicit parentheses assumed
            get<VS>(se.data).push_back(
                lexConfigTokensList(tokens, tokenIter, 1, true)
            );
        }
        else if (tokenIter->type == ConfigFileToken::Type::parenthesisLeft) {
            get<VS>(se.data).push_back(
                lexConfigTokensList(tokens, ++tokenIter, 1, false)
            );
        }
        else if (tokenIter->type == ConfigFileToken::Type::parenthesisRight) {
            LOG(ERROR) << "Unexpected ')'.";
            throw runtime_error("Unmatched parentheses");
        }
        else {
            ++tokenIter;
        }

    }

    return se;
}

// Build tokens from s-expression
// No comment or line break will be created
inline list< ConfigFileToken > buildConfigTokens(const SExpr& se) {
    using namespace std;
    using TokenList = list< ConfigFileToken >;

    TokenList tokens;

    struct TokenBuildVisitor {
        TokenList& theList;
        void operator()(const SExpr::StringType& str) const {
            theList.push_back(ConfigFileToken::makeString( str ));
        }
        void operator()(const SExpr::ListType& seList) const {
            theList.push_back(ConfigFileToken::makeDefault(ConfigFileToken::Type::parenthesisLeft));
            for(const auto& eachSE : seList) {
                visit(TokenBuildVisitor{theList}, eachSE.data);
            }
            theList.push_back(ConfigFileToken::makeDefault(ConfigFileToken::Type::parenthesisRight));
        }
    };

    visit(TokenBuildVisitor{ tokens }, se.data);
    return tokens;
}


// Key-value parser
//
// Treats an input s-expression as a list of key-value pairs and parse by
// matching keywords
//
// Features:
//   - Convert system input (s-expression) to params
//   - Build formatted tokens (for output) from params
template< typename Params >
struct KeyValueParser {

    enum class UnknownKeyAction {
        ignore, warn, error
    };
    using ParserFunc = std::function< void(Params&, const SExpr::ListType&) >;
    using TokenBuildFunc = std::function< std::list< ConfigFileToken >(const Params&) >;

    // Two data structures are required for the parser to work.
    //   - A dictionary to match keywords to the appropriate function for
    //     parsing the arguments
    //   - A list which instructs how an input file should be generated from
    //     system params
    std::map< std::string, ParserFunc > dict;
    std::vector< TokenBuildFunc > tokenBuildList;

    void parse(
        Params&          params,
        const SExpr&     se,
        UnknownKeyAction unknownKeyAction = UnknownKeyAction::ignore
    ) const {
        using namespace std;
        using SES = SExpr::StringType;
        using SEL = SExpr::ListType;

        // se must be a list type, or an exception will be thrown
        for(const SExpr& eachList : get<SEL>(se.data)) {
            // eachList.data must be a list type, or an exception will be thrown
            // eachList contains the (key arg1 arg2 ...) data

            const SES key = get<SES>(car(eachList).data);
            const SEL keyAndArgs = get<SEL>(eachList.data);

            // Check against the parsing dictionary
            if(auto it = dict.find(key); it == dict.end()) {
                switch(unknownKeyAction) {
                case UnknownKeyAction::ignore:
                    break;
                case UnknownKeyAction::warn:
                    LOG(WARNING) << "In the input file, "
                        << key << " cannot be recognized.";
                    break;
                case UnknownKeyAction::error:
                    LOG(ERROR) << "In the input file, "
                        << key << " cannot be recognized.";
                    throw runtime_error("Unknown key in parser");
                }
            }
            else {
                // Execute the settings
                (it->second)(params, keyAndArgs);
            }
        }
    }

    auto buildTokens(const Params& params) const {
        std::list< ConfigFileToken > res;

        for(const auto& eachTokenBuild : tokenBuildList) {
            res.push_back(ConfigFileToken::makeDefault(ConfigFileToken::Type::parenthesisLeft));
            res.splice(res.end(), eachTokenBuild(params));
            res.push_back(ConfigFileToken::makeDefault(ConfigFileToken::Type::parenthesisRight));
        }
        return res;
    }

    // Handy functions
    //---------------------------------

    // Template parameters
    //   - FuncParse: void(Params&, const vector<string>&), including key
    //   - FuncBuild:
    //     vector<string>(const Param&), excluding key, OR
    //     vector<vector<string>>(const Param&), excluding key (for repeating items)
    template<
        typename FuncParse,
        typename FuncBuild
    >
    void addStringArgs(
        std::string name,
        FuncParse&& funcParse,
        FuncBuild&& funcBuild
    ) {
        using namespace std;

        return addStringArgsWithAliases(
            move(name),
            {},
            forward<FuncParse>(funcParse),
            forward<FuncBuild>(funcBuild)
        );
    }

    // With alias
    // Template parameters
    //   - FuncParse: void(Params&, const vector<string>&), including key
    //   - FuncBuild:
    //     vector<string>(const Param&), excluding key, OR
    //     vector<vector<string>>(const Param&), excluding key (for repeating items)
    template<
        typename FuncParse,
        typename FuncBuild
    >
    void addStringArgsWithAliases(
        std::string name,
        std::vector< std::string > aliases,
        FuncParse&& funcParse,
        FuncBuild&& funcBuild
    ) {
        using namespace std;

        const auto insertResult = dict.insert({
            name,
            [funcParse] (Params& params, const SExpr::ListType& keyAndArgs) {
                funcParse(params, getStringVector(keyAndArgs));
            }
        });
        // Insert for aliases
        for(auto& alias : aliases) {
            dict.insert({ move(alias), insertResult.first->second });
        }

        tokenBuildList.push_back(
            [funcBuild, name] (const Params& params) {

                list< ConfigFileToken > res;

                if constexpr(
                    is_same_v< invoke_result_t< FuncBuild, const Params& >, vector<string> >
                ) {
                    res.push_back(ConfigFileToken::makeString(name));

                    vector<string> tempResult = funcBuild(params);
                    for(auto& s : tempResult) {
                        res.push_back(ConfigFileToken::makeString(move(s)));
                    }

                    res.push_back(ConfigFileToken::makeDefault(ConfigFileToken::Type::lineBreak));

                } else {
                    vector<vector<string>> tempResult = funcBuild(params);
                    for(auto& eachVS : tempResult) {
                        res.push_back(ConfigFileToken::makeString(name));

                        for(auto& s : eachVS) {
                            res.push_back(ConfigFileToken::makeString(move(s)));
                        }

                        res.push_back(ConfigFileToken::makeDefault(ConfigFileToken::Type::lineBreak));
                    }
                }

                return res;
            }
        );
    }

    void addComment(std::string comment) {
        // The comment must start with valid comment specifiers such as ';'
        tokenBuildList.push_back(
            [comment{ std::move(comment) }] (const Params&) {
                std::list< ConfigFileToken > res {
                    ConfigFileToken {
                        ConfigFileToken::Type::comment,
                        comment
                    }
                };
                res.push_back(ConfigFileToken::makeDefault(ConfigFileToken::Type::lineBreak));
                return res;
            }
        );
    }
    void addEmptyLine() {
        tokenBuildList.push_back(
            [] (const Params&) {
                return std::list< ConfigFileToken > {
                    ConfigFileToken::makeDefault(ConfigFileToken::Type::lineBreak)
                };
            }
        );
    }

};

struct SystemParser {
    // Preferably, for one input file, only one parser is needed.
    // However, currently, some parsing rules require the results from parsed
    // params in the same file, and therefore multiple parsing passes are
    // needed.
    // So it is natural to write each set of parsing rules in a separate
    // parser.
    KeyValueParser< SimulConfig >
        headerParser,
        geoParser,
        boundParser,
        mechParser;

    SystemParser() {
        initInputHeader();
        initGeoParser();
        initBoundParser();
        initMechParser();
        // TODO
    }

    void parseSystemInput(SimulConfig& conf, std::string input) const {
        const auto se =
            lexConfigTokens(
                tokenizeConfigFile(
                    std::move(input)));

        geoParser.parse(conf, se);
        geoPostProcessing(conf);

        boundParser.parse(conf, se);
        boundPostProcessing(conf);

        mechParser.parse(conf, se);

    }
    void outputSystemInput(std::ostream& os, const SimulConfig& conf) const {
        std::list< ConfigFileToken > tokens;
        tokens.splice(tokens.end(), headerParser.buildTokens(conf));
        tokens.splice(tokens.end(), geoParser.buildTokens(conf));
        tokens.splice(tokens.end(), boundParser.buildTokens(conf));
        tokens.splice(tokens.end(), mechParser.buildTokens(conf));

        outputConfigTokens(os, tokens);
    }

    // Add parsing rules
    void initInputHeader();
    void initGeoParser();
    void initBoundParser();
    void initMechParser();

    // Post processing and validation
    void geoPostProcessing(SimulConfig&) const;
    void boundPostProcessing(SimulConfig&) const;

};

} // namespace medyan

/// A general parser
/*!
 *  A parser object, when initialized, opens an input file. Upon destruction, it 
 *  closes the file.
 */
class Parser {
protected:
    fstream _inputFile; ///< input file being used
    
public:
    Parser(string inputFileName) {
        _inputFile.open(inputFileName);
        if(!_inputFile.is_open()) {
            cout << "There was an error parsing file " << inputFileName
                 << ". Exiting." << endl;
            exit(EXIT_FAILURE);
        }
        cout << "Loading file " << inputFileName << endl;
    }
    ~Parser() {_inputFile.close();}
};

/// To parse a system input file, initialized by the Controller.
struct SystemParser {
    
    //@{
    /// Parameter parser. Reads input directly into system parameters
    /// @note - does not check for correctness and consistency here.
    static ChemParams    readChemParams(std::istream&, const GeoParams&);
    static DyRateParams  readDyRateParams(std::istream&);
    static SpecialParams readSpecialParams(std::istream&);
    //@}
    
    //@{
    /// Algorithm parser
    static ChemParams::ChemistryAlgorithm readChemistryAlgorithm(std::istream&);
    //@}
    
    //@{
    /// Type parser
    static DyRateParams::DynamicRateType   readDynamicRateType(std::istream&);
    static SpecialParams::SpecialSetupType readSpecialSetupType(std::istream&);
    //@}
    
    /// Read Filament information
    static FilamentSetup readFilamentSetup(std::istream&);
    
    /// Read Bubble information
    static BubbleSetup readBubbleSetup(std::istream&);
    
    /// Chemistry information
    static ChemParams::ChemistrySetup readChemistrySetup(std::istream&);
};

/// Used to parse initial Filament information, initialized by the Controller.
struct FilamentParser {
  
    /// Reads filament input file. Returns a vector of tuples containing
    /// filament type and positions (start and end points).
    /// @note - Does not check for coordinate correctness.
    static FilamentData readFilaments(std::istream&);
};

/// Used to parse initial Bubble information, initialized by the Controller.
struct BubbleParser {    
    /// Reads bubble input file. Returns a vector of tuples containing
    /// bubble type and position.
    /// @note - Does not check for coordinate correctness.
    static BubbleData readBubbles(std::istream&);
};


/// Used to parse all chemical information, initialized by the Controller.
struct ChemistryParser {
    /// Reads chemical reactions and species from input file. Returns a
    /// ChemistryData struct containing this data
    /// @note - this does not check for consistency and correctness, the only
    ///         sanity check here is that there are no duplicate species names.
    static ChemistryData readChemistryInput(std::istream&, const ChemParams&);
};


/// Used to parse pin positions if needed upon restart
class PinRestartParser: public Parser {
    
public:
    PinRestartParser(string inputFileName) : Parser(inputFileName) {}
    ~PinRestartParser() {}
    
    /// Reads pin positions from file, and sets filaments
    void resetPins();
};



#endif
