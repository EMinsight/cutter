/** \file CutterDescriptions.h
 * This file contains every structure description that are used in widgets.
 * The descriptions are used for the Qt metatypes.
 */
#ifndef DESCRIPTIONS_H
#define DESCRIPTIONS_H

#include <QString>
#include <QList>
#include <QStringList>
#include <QMetaType>
#include <QColor>
#include "core/CutterCommon.h"

struct FunctionDescription
{
    RVA offset;
    RVA linearSize;
    RVA nargs;
    RVA nbbs;
    RVA nlocals;
    QString calltype;
    QString name;
    RVA edges;
    RVA stackframe;

    bool contains(RVA addr) const
    {
        // TODO: this is not exactly correct in edge cases.
        // rz_analysis_function_contains() does it right.
        return addr >= offset && addr < offset + linearSize;
    }
};

struct ImportDescription
{
    RVA plt;
    int ordinal;
    QString bind;
    QString type;
    QString name;
    QString libname;
};

struct ExportDescription
{
    RVA vaddr;
    RVA paddr;
    RVA size;
    QString type;
    QString name;
    QString flag_name;
};

struct HeaderDescription
{
    RVA vaddr;
    RVA paddr;
    QString value;
    QString name;
};

struct FlirtDescription
{
    QString bin_name;
    QString arch_name;
    QString arch_bits;
    QString base_name;
    QString short_path;
    QString file_path;
    QString details;
    QString n_modules;
};

struct TypeDescription
{
    QString type;
    int size;
    QString format;
    QString category;
};

struct SearchDescription
{
    RVA offset;
    size_t size;
    QString code;
    QString data;
    QString detail;
};

struct SymbolDescription
{
    RVA vaddr;
    QString bind;
    QString type;
    QString name;
};

struct CommentDescription
{
    RVA offset;
    QString name;
};

struct RelocDescription
{
    RVA vaddr;
    RVA paddr;
    QString type;
    QString name;
};

struct StringDescription
{
    RVA vaddr;
    QString string;
    QString type;
    QString section;
    ut32 length;
    ut32 size;
};

struct FlagspaceDescription
{
    QString name;
};

struct FlagDescription
{
    RVA offset;
    RVA size;
    QString name;
    QString realname;
};

struct SectionDescription
{
    RVA vaddr;
    RVA paddr;
    RVA size;
    RVA vsize;
    QString name;
    QString perm;
    QString entropy;
};

struct SegmentDescription
{
    RVA vaddr;
    RVA paddr;
    RVA size;
    RVA vsize;
    QString name;
    QString perm;
};

struct EntrypointDescription
{
    RVA vaddr;
    RVA paddr;
    RVA baddr;
    RVA laddr;
    RVA haddr;
    QString type;
};

struct XrefDescription
{
    RVA from;
    QString from_str;
    RVA to;
    QString to_str;
    QString type;
};

struct RzBinPluginDescription
{
    QString name;
    QString description;
    QString license;
    QString type;
};

struct RzIOPluginDescription
{
    QString name;
    QString description;
    QString license;
    QString permissions;
    QList<QString> uris;
};

struct RzCorePluginDescription
{
    QString name;
    QString description;
    QString license;
};

struct RzAsmPluginDescription
{
    QString name;
    QString architecture;
    QString author;
    QString version;
    QString cpus;
    QString description;
    QString license;
};

struct DisassemblyLine
{
    RVA offset;
    QString text;
    RVA arrow;
};

struct BinClassBaseClassDescription
{
    QString name;
    RVA offset;
};

struct BinClassMethodDescription
{
    QString name;
    RVA addr = RVA_INVALID;
    st64 vtableOffset = -1;
};

struct BinClassFieldDescription
{
    QString name;
    RVA addr = RVA_INVALID;
};

struct BinClassDescription
{
    QString name;
    RVA addr = RVA_INVALID;
    RVA vtableAddr = RVA_INVALID;
    QList<BinClassBaseClassDescription> baseClasses;
    QList<BinClassMethodDescription> methods;
    QList<BinClassFieldDescription> fields;
};

struct AnalysisMethodDescription
{
    QString name;
    QString realName;
    RVA addr;
    st64 vtableOffset;
};

struct AnalysisBaseClassDescription
{
    QString id;
    RVA offset;
    QString className;
};

struct AnalysisVTableDescription
{
    QString id;
    ut64 offset;
    ut64 addr;
};

struct ResourcesDescription
{
    QString name;
    RVA vaddr;
    ut64 index;
    QString type;
    ut64 size;
    QString lang;
};

struct VTableDescription
{
    RVA addr;
    QList<BinClassMethodDescription> methods;
};

struct BlockDescription
{
    RVA addr;
    RVA size;
    int flags;
    int functions;
    int inFunctions;
    int comments;
    int symbols;
    int strings;
    ut8 rwx;
};

struct BlockStatistics
{
    RVA from;
    RVA to;
    RVA blocksize;
    QList<BlockDescription> blocks;
};

struct MemoryMapDescription
{
    RVA addrStart;
    RVA addrEnd;
    QString name;
    QString fileName;
    QString type;
    QString permission;
};

struct BreakpointDescription
{
    enum PositionType {
        Address,
        Named,
        Module,
    };

    RVA addr = 0;
    int64_t moduleDelta = 0;
    int index = -1;
    PositionType type = Address;
    int size = 0;
    int permission = 0;
    QString positionExpression;
    QString name;
    QString command;
    QString condition;
    bool hw = false;
    bool trace = false;
    bool enabled = true;
};

struct ProcessDescription
{
    bool current;
    int pid;
    int uid;
    int ppid;
    RzDebugPidState status;
    QString path;
};

struct ThreadDescription
{
    bool current;
    int pid;
    int uid;
    int ppid;
    RzDebugPidState status;
    QString path;
    ut64 pc;
    ut64 tls;
};

struct RefDescription
{
    QString ref;
    QColor refColor;
};

struct VariableDescription
{
    RzAnalysisVarStorageType storageType;
    QString name;
    QString type;
    QString value;
};

struct GlobalDescription
{
    RVA addr;
    QString type;
    QString name;
};

struct RegisterRefValueDescription
{
    QString name;
    QString value;
    QString ref;
};

struct Chunk
{
    RVA offset;
    QString status;
    int size;
};

struct Arena
{
    RVA offset;
    QString type;
    ut64 top;
    ut64 last_remainder;
    ut64 next;
    ut64 next_free;
    ut64 system_mem;
    ut64 max_system_mem;
};

struct BasefindCoreStatusDescription
{
    size_t index;
    ut32 percentage;
};

struct BasefindResultDescription
{
    RVA candidate;
    ut32 score;
};

Q_DECLARE_METATYPE(FunctionDescription)
Q_DECLARE_METATYPE(ImportDescription)
Q_DECLARE_METATYPE(ExportDescription)
Q_DECLARE_METATYPE(SymbolDescription)
Q_DECLARE_METATYPE(CommentDescription)
Q_DECLARE_METATYPE(RelocDescription)
Q_DECLARE_METATYPE(StringDescription)
Q_DECLARE_METATYPE(FlagspaceDescription)
Q_DECLARE_METATYPE(FlagDescription)
Q_DECLARE_METATYPE(GlobalDescription)
Q_DECLARE_METATYPE(XrefDescription)
Q_DECLARE_METATYPE(EntrypointDescription)
Q_DECLARE_METATYPE(RzBinPluginDescription)
Q_DECLARE_METATYPE(RzIOPluginDescription)
Q_DECLARE_METATYPE(RzCorePluginDescription)
Q_DECLARE_METATYPE(RzAsmPluginDescription)
Q_DECLARE_METATYPE(BinClassMethodDescription)
Q_DECLARE_METATYPE(BinClassFieldDescription)
Q_DECLARE_METATYPE(BinClassDescription)
Q_DECLARE_METATYPE(const BinClassDescription *)
Q_DECLARE_METATYPE(const BinClassMethodDescription *)
Q_DECLARE_METATYPE(const BinClassFieldDescription *)
Q_DECLARE_METATYPE(AnalysisBaseClassDescription)
Q_DECLARE_METATYPE(AnalysisMethodDescription)
Q_DECLARE_METATYPE(AnalysisVTableDescription)
Q_DECLARE_METATYPE(ResourcesDescription)
Q_DECLARE_METATYPE(VTableDescription)
Q_DECLARE_METATYPE(TypeDescription)
Q_DECLARE_METATYPE(HeaderDescription)
Q_DECLARE_METATYPE(FlirtDescription)
Q_DECLARE_METATYPE(SearchDescription)
Q_DECLARE_METATYPE(SectionDescription)
Q_DECLARE_METATYPE(SegmentDescription)
Q_DECLARE_METATYPE(MemoryMapDescription)
Q_DECLARE_METATYPE(BreakpointDescription)
Q_DECLARE_METATYPE(BreakpointDescription::PositionType)
Q_DECLARE_METATYPE(ProcessDescription)
Q_DECLARE_METATYPE(RefDescription)
Q_DECLARE_METATYPE(VariableDescription)
Q_DECLARE_METATYPE(BasefindCoreStatusDescription)
Q_DECLARE_METATYPE(BasefindResultDescription)

#endif // DESCRIPTIONS_H
