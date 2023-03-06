#pragma once

#include "json_utils.h"

#include <map>
#include <optional>
#include <vector>

namespace Core {

class CppDocument;

//! Store ToggleSection settings
struct ToggleSectionSettings
{
    QString tag;
    QString debug;
    std::map<std::string, std::string> return_values;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ToggleSectionSettings, tag, debug, return_values);

class IncludeHelper
{
public:
    explicit IncludeHelper(CppDocument *document);

    struct IncludePosition
    {
        int line = -1; // line is 0-based
        bool newGroup = false;
        bool alreadyExists() const { return line == -1; }
    };
    /**
     * Returns the line (1-based) the new include should be inserted
     * addNewGroup is set to true if it needs a new group, or false otherwise.
     * If addNewGroup is true when calling the method, it will add the include at the end
     */
    std::optional<IncludePosition> includePositionForInsertion(const QString &text, bool addNewGroup);

    /**
     * Returns the line (1-based) the include should be removed.
     */
    std::optional<int> includePositionForRemoval(const QString &text);

private:
    struct Include
    {
        enum Scope {
            Local = 0x1,
            Global = 0x2,
            Mixed = Local | Global,
        };
        QString name;
        Scope scope;
        int line; // line are 1-based
        bool isNull() const { return name.isEmpty(); }
        bool operator==(const Include &other) const;
    };
    using Includes = std::vector<Include>;
    struct IncludeGroup
    {
        int first;
        int last;
        int lastLine; // line are 1-based
        QString prefix;
        int scope = 0;
    };
    using IncludeGroups = std::vector<IncludeHelper::IncludeGroup>;

    /**
     * Returns an Include struct based on the name, the name should be '<foo.h>' or '"foo.h"'
     */
    Include includeForText(const QString &text) const;
    /**
     * Find the position of the include
     */
    Includes::const_iterator findInclude(const Include &include) const;

    /**
     * Find the best position for inserting an include
     */
    IncludeGroups::const_iterator findBestIncludeGroup(const Include &include) const;
    /**
     * Find best line for include if there are no includes
     */
    IncludePosition findBestFirstIncludeLine() const;

    /**
     * Compute all includes and include groups in the file
     */
    void computeIncludes();

    CppDocument *const m_document;
    Includes m_includes;
    IncludeGroups m_includeGroups;
};

} // namespace Core
