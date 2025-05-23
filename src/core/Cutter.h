#ifndef CUTTER_H
#define CUTTER_H

#include "core/CutterCommon.h"
#include "core/CutterDescriptions.h"
#include "core/CutterJson.h"
#include "core/Basefind.h"
#include "common/BasicInstructionHighlighter.h"

#include <QMap>
#include <QMenu>
#include <QDebug>
#include <QObject>
#include <QSharedPointer>
#include <QStringList>
#include <QMessageBox>
#include <QErrorMessage>
#include <QMutex>
#include <QDir>
#include <functional>
#include <memory>

class AsyncTaskManager;
class BasicInstructionHighlighter;
class CutterCore;
class Decompiler;
class RizinTask;
class RizinCmdTask;
class RizinFunctionTask;
class RizinTaskDialog;

#include "common/BasicBlockHighlighter.h"
#include "common/Helpers.h"

#include <rz_project.h>
#include <memory>

#define Core() (CutterCore::instance())

class RzCoreLocked;

struct CUTTER_EXPORT AddrRefs
{
    RVA addr;
    QString mapname;
    QString section;
    QString reg;
    QString fcn;
    QString type;
    QString asm_op;
    QString perms;
    ut64 value;
    bool has_value;
    QString string;
    QSharedPointer<AddrRefs> ref;
};

struct CUTTER_EXPORT RegisterRef
{
    ut64 value;
    AddrRefs ref;
    QString name;
};

enum class SearchKind {
    AsmCode,
    HexString,
    ROPGadgets,
    ROPGadgetsRegex,
    String,
    StringCaseInsensitive,
    StringRegexExtended,
    Value32BE,
    Value32LE,
    Value64BE,
    Value64LE,
    CryptographicMaterial,
    MagicSignature,
};

class CUTTER_EXPORT CutterCore : public QObject
{
    Q_OBJECT

    friend class RzCoreLocked;
    friend class RizinTask;
    friend class Basefind;

public:
    explicit CutterCore(QObject *parent = nullptr);
    ~CutterCore();
    static CutterCore *instance();

    void initialize(bool loadPlugins = true);
    void loadCutterRC();
    void loadDefaultCutterRC();
    QDir getCutterRCDefaultDirectory() const;

    AsyncTaskManager *getAsyncTaskManager() { return asyncTaskManager; }

    RVA getOffset() const { return core_->offset; }

    /* Core functions (commands) */
    /* Almost the same as core_cmd_raw,
     * only executes std::function<bool(RzCore *)> instead of char* */
    QString getFunctionExecOut(const std::function<bool(RzCore *)> &fcn,
                               const RVA addr = RVA_INVALID);
    static QString sanitizeStringForCommand(QString s);
    /**
     * @brief send a command to Rizin
     * @param str the command you want to execute
     * @return command output
     * @note if you want to seek to an address, you should use CutterCore::seek.
     */
    QString cmd(const char *str);
    QString cmd(const QString &str) { return cmd(str.toUtf8().constData()); }

    /**
     * @brief send a task to Rizin
     * @param fcn the task you want to execute
     * @return execute successful?
     */
    bool asyncTask(std::function<void *(RzCore *)> fcn, QSharedPointer<RizinTask> &task);
    void functionTask(std::function<void *(RzCore *)> fcn);

    /**
     * @brief Execute a Rizin command \a cmd.  By nature, the API
     * is executing raw commands, and thus ignores multiple commands and overcome command
     * injections.
     * @param cmd - a raw command to execute. Passing multiple commands (e.g "px 5; pd 7 && pdf")
     * will result in them treated as arguments to first command.
     * @return the output of the command
     */
    QString cmdRaw(const char *cmd);

    /**
     * @brief a wrapper around cmdRaw(const char *cmd,).
     */
    QString cmdRaw(const QString &cmd) { return cmdRaw(cmd.toUtf8().constData()); };

    /**
     * @brief Execute a Rizin command \a cmd at \a address. The function will preform a silent seek
     * to the address without triggering the seekChanged event nor adding new entries to the seek
     * history. By nature, the API is executing a single command without going through Rizin shell,
     * and thus ignores multiple commands and tries to overcome command injections.
     * @param cmd - a raw command to execute. If multiple commands will be passed (e.g "px 5; pd 7
     * && pdf") then only the first command will be executed.
     * @param address - an address to which Cutter will temporarily seek.
     * @return the output of the command
     */
    QString cmdRawAt(const char *cmd, RVA address);

    /**
     * @brief a wrapper around cmdRawAt(const char *cmd, RVA address).
     */
    QString cmdRawAt(const QString &str, RVA address)
    {
        return cmdRawAt(str.toUtf8().constData(), address);
    }

    class SeekReturn
    {
        RVA returnAddress;
        bool empty = true;

    public:
        SeekReturn(RVA returnAddress) : returnAddress(returnAddress), empty(false) {}
        ~SeekReturn()
        {
            if (!empty) {
                Core()->seekSilent(returnAddress);
            }
        }
        SeekReturn(SeekReturn &&from)
        {
            if (this != &from) {
                returnAddress = from.returnAddress;
                empty = from.empty;
                from.empty = true;
            }
        };
    };

    SeekReturn seekTemp(RVA address)
    {
        SeekReturn returner(getOffset());
        seekSilent(address);
        return returner;
    }

    enum class SeekHistoryType { New, Undo, Redo };

    CutterJson cmdj(const char *str);
    CutterJson cmdj(const QString &str) { return cmdj(str.toUtf8().constData()); }
    QString cmdTask(const QString &str);

    QString getRizinVersionReadable(const char *program = nullptr);
    QString getVersionInformation();

    CutterJson parseJson(const char *name, char *res, const char *cmd = nullptr);
    CutterJson parseJson(const char *name, char *res, const QString &cmd = QString())
    {
        return parseJson(name, res, cmd.isNull() ? nullptr : cmd.toLocal8Bit().constData());
    }

    QStringList autocomplete(const QString &cmd, RzLinePromptType promptType);

    /* Functions methods */
    void renameFunction(const RVA offset, const QString &newName);
    void delFunction(RVA addr);
    void renameFlag(QString old_name, QString new_name);
    /**
     * @brief Renames the specified local variable in the function specified by the
     * address given.
     * @param newName Specifies the name to which the current name of the variable
     * should be renamed.
     * @param oldName Specifies the current name of the function variable.
     * @param functionAddress Specifies the exact address of the function.
     */
    void renameFunctionVariable(QString newName, QString oldName, RVA functionAddress);

    /**
     * @param addr
     * @return a function that contains addr or nullptr
     */
    RzAnalysisFunction *functionIn(ut64 addr);

    /**
     * @param addr
     * @return the function that has its entrypoint at addr or nullptr
     */
    RzAnalysisFunction *functionAt(ut64 addr);

    RVA getFunctionStart(RVA addr);
    RVA getFunctionEnd(RVA addr);
    RVA getLastFunctionInstruction(RVA addr);
    QString flagAt(RVA addr, bool getClosestFlag = true);
    void createFunctionAt(RVA addr);
    void createFunctionAt(RVA addr, QString name);
    QStringList getDisassemblyPreview(RVA address, int num_of_lines);

    /* Flags */
    void delFlag(RVA addr);
    void delFlag(const QString &name);
    void addFlag(RVA offset, QString name, RVA size);
    QString listFlagsAsStringAt(RVA addr);
    /**
     * @brief Get nearest flag at or before offset.
     * @param offset search position
     * @param flagOffsetOut address of returned flag
     * @return flag name
     */
    QString nearestFlag(RVA offset, RVA *flagOffsetOut);
    void triggerFlagsChanged();

    /* Global Variables */
    void addGlobalVariable(RVA offset, QString name, QString typ);
    void delGlobalVariable(QString name);
    void delGlobalVariable(RVA offset);
    void modifyGlobalVariable(RVA offset, QString name, QString typ);
    QString getGlobalVariableType(QString name);
    QString getGlobalVariableType(RVA offset);

    /* Edition functions */
    CutterRzIter<RzAnalysisBytes> getRzAnalysisBytesSingle(RVA addr);
    QString getInstructionBytes(RVA addr);
    QString getInstructionOpcode(RVA addr);
    void editInstruction(RVA addr, const QString &inst, bool fillWithNops = false);
    void nopInstruction(RVA addr);
    void jmpReverse(RVA addr);
    void editBytes(RVA addr, const QString &inst);
    void editBytesEndian(RVA addr, const QString &bytes);

    /* Code/Data */
    void setToCode(RVA addr);
    enum class StringTypeFormats { None, ASCII_LATIN1, UTF8 };
    /**
     * @brief Adds string at address
     * That function calls the 'Cs' command
     * \param addr The address of the array where the string will be applied
     * \param size The size of string
     * \param type The type of string
     */
    void setAsString(RVA addr, int size = 0, StringTypeFormats type = StringTypeFormats::None);
    /**
     * @brief Removes string at address
     * That function calls the 'Cs-' command
     * \param addr The address of the array where the string will be applied
     */
    void removeString(RVA addr);
    /**
     * @brief Gets string at address
     * That function correspond the 'Cs.' command
     * \param addr The address of the string
     * @return string at requested address
     */
    QString getMetaString(RVA addr);
    /**
     * @brief Gets string at address
     * That function calls the 'ps' command
     * \param addr The address of the first byte of the array
     * @return string at requested address
     */
    QString getString(RVA addr);
    QString getString(RVA addr, uint64_t len, RzStrEnc encoding, bool escape_nl = false);
    void setToData(RVA addr, int size, int repeat = 1);
    int sizeofDataMeta(RVA addr);

    /* Comments */
    void setComment(RVA addr, const QString &cmt);
    void delComment(RVA addr);
    QString getCommentAt(RVA addr);
    void setImmediateBase(const QString &rzBaseName, RVA offset = RVA_INVALID);
    void setCurrentBits(int bits, RVA offset = RVA_INVALID);

    /**
     * @brief Changes immediate displacement to structure offset
     * This function makes use of the "aht" command of Rizin to apply structure
     * offset to the immediate displacement used in the given instruction
     * \param structureOffset The name of struct which will be applied
     * \param offset The address of the instruction where the struct will be applied
     */
    void applyStructureOffset(const QString &structureOffset, RVA offset = RVA_INVALID);

    /* Classes */
    QList<QString> getAllAnalysisClasses(bool sorted);
    QList<AnalysisMethodDescription> getAnalysisClassMethods(const QString &cls);
    QList<AnalysisBaseClassDescription> getAnalysisClassBaseClasses(const QString &cls);
    QList<AnalysisVTableDescription> getAnalysisClassVTables(const QString &cls);
    void createNewClass(const QString &cls);
    void renameClass(const QString &oldName, const QString &newName);
    void deleteClass(const QString &cls);
    bool getAnalysisMethod(const QString &cls, const QString &meth,
                           AnalysisMethodDescription *desc);
    void renameAnalysisMethod(const QString &className, const QString &oldMethodName,
                              const QString &newMethodName);
    void setAnalysisMethod(const QString &cls, const AnalysisMethodDescription &meth);

    /* File related methods */
    bool loadFile(QString path, ut64 baddr = 0LL, ut64 mapaddr = 0LL, int perms = RZ_PERM_R,
                  int va = 0, bool loadbin = false, const QString &forceBinPlugin = QString());
    bool tryFile(QString path, bool rw);
    bool mapFile(QString path, RVA mapaddr);
    void loadScript(const QString &scriptname);

    /* Seek functions */
    void seek(QString thing);
    void seek(ut64 offset);
    void seekSilent(ut64 offset);
    void seekSilent(QString thing) { seekSilent(math(thing)); }
    void seekPrev();
    void seekNext();
    void updateSeek(SeekHistoryType type = SeekHistoryType::New);
    /**
     * @brief Raise a memory widget showing current offset, prefer last active
     * memory widget.
     */
    void showMemoryWidget();
    /**
     * @brief Seek to \p offset and raise a memory widget showing it.
     * @param offset
     */
    void seekAndShow(ut64 offset);
    /**
     * @brief \see CutterCore::show(ut64)
     * @param thing - addressable expression
     */
    void seekAndShow(QString thing);
    RVA getOffset();
    RVA prevOpAddr(RVA startAddr, int count);
    RVA nextOpAddr(RVA startAddr, int count);

    /* SigDB / Flirt functions */
    void applySignature(const QString &filepath);
    void createSignature(const QString &filepath);

    /* Math functions */
    ut64 math(const QString &expr);
    ut64 num(const QString &expr);
    QString itoa(ut64 num, int rdx = 16);

    /* Config functions */
    void setConfig(const char *k, const char *v);
    void setConfig(const QString &k, const char *v);
    void setConfig(const char *k, const QString &v);
    void setConfig(const QString &k, const QString &v) { setConfig(k.toUtf8().constData(), v); }
    void setConfig(const char *k, int v);
    void setConfig(const QString &k, int v) { setConfig(k.toUtf8().constData(), v); }
    void setConfig(const char *k, bool v);
    void setConfig(const QString &k, bool v) { setConfig(k.toUtf8().constData(), v); }
    void setConfig(const char *k, const QVariant &v);
    void setConfig(const QString &k, const QVariant &v) { setConfig(k.toUtf8().constData(), v); }
    int getConfigi(const char *k);
    int getConfigi(const QString &k) { return getConfigi(k.toUtf8().constData()); }
    ut64 getConfigut64(const char *k);
    ut64 getConfigut64(const QString &k) { return getConfigut64(k.toUtf8().constData()); }
    bool getConfigb(const char *k);
    bool getConfigb(const QString &k) { return getConfigb(k.toUtf8().constData()); }
    QString getConfig(const char *k);
    QString getConfig(const QString &k) { return getConfig(k.toUtf8().constData()); }
    QString getConfigDescription(const char *k);
    QStringList getConfigOptions(const char *k);
    QStringList getColorThemes();
    QHash<QString, QColor> getTheme();
    QStringList getThemeKeys();
    bool setColor(const QString &key, const QString &color);
    QStringList getConfigVariableSpaces(const QString &key = "");

    /* Assembly\Hexdump related methods */
    QByteArray assemble(const QString &code);
    QString disassemble(const QByteArray &data);
    QString disassembleSingleInstruction(RVA addr);
    QList<DisassemblyLine> disassembleLines(RVA offset, int lines);

    static QByteArray hexStringToBytes(const QString &hex);
    static QString bytesToHexString(const QByteArray &bytes);
    enum class HexdumpFormats { Normal, Half, Word, Quad, Signed, Octal };
    QString hexdump(RVA offset, int size, HexdumpFormats format);
    QString getHexdumpPreview(RVA offset, int size);

    void setCPU(QString arch, QString cpu, int bits);
    void setEndianness(bool big);

    /* SDB */
    QList<QString> sdbList(QString path);
    QList<QString> sdbListKeys(QString path);
    QString sdbGet(QString path, QString key);
    bool sdbSet(QString path, QString key, QString val);

    /* Debug */
    QString getRegisterName(QString registerRole);
    RVA getProgramCounterValue();
    void setRegister(QString regName, QString regValue);
    void setCurrentDebugThread(int tid);
    /**
     * @brief Attach to a given pid from a debug session
     */
    void setCurrentDebugProcess(int pid);
    /**
     * @brief Returns a list of stack address and their telescoped references
     * @param size number of bytes to scan
     * @param depth telescoping depth
     */
    QList<AddrRefs> getStack(int size = 0x100, int depth = 6);
    /**
     * @brief Recursively dereferences pointers starting at the specified address
     *        up to a given depth
     * @param addr telescoping addr
     * @param depth telescoping depth
     */
    AddrRefs getAddrRefs(RVA addr, int depth);
    /**
     * @brief return a RefDescription with a formatted ref string and configured colors
     * @param ref the "ref" JSON node from getAddrRefs
     */
    RefDescription formatRefDesc(const QSharedPointer<AddrRefs> &ref);
    /**
     * @brief Get a list of a given process's threads
     * @param pid The pid of the process, -1 for the currently debugged process
     * @return List of ProcessDescription
     */
    QList<ThreadDescription> getProcessThreads(int pid = -1);
    /**
     * @brief Get a list of heap chunks
     * Uses RZ_API rz_heap_chunks_list to get vector of chunks
     * If arena_addr is zero return the chunks for main arena
     * @param arena_addr base address for the arena
     * @return Vector of heap chunks for the given arena
     */
    QVector<Chunk> getHeapChunks(RVA arena_addr);

    /**
     * @brief Get a list of heap arenas
     * Uses RZ_API rz_heap_arenas_list to get list of arenas
     * @return Vector of arenas
     */
    QVector<Arena> getArenas();

    /**
     * @brief Get detailed information about a heap chunk
     * Uses RZ_API rz_heap_chunk
     * @return RzHeapChunkSimple struct pointer for the heap chunk
     */
    RzHeapChunkSimple *getHeapChunk(ut64 addr);
    /**
     * @brief Get heap bins of an arena with given base address
     * (including large, small, fast, unsorted, tcache)
     * @param arena_addr Base address of the arena
     * @return QVector of non empty RzHeapBin pointers
     */
    QVector<RzHeapBin *> getHeapBins(ut64 arena_addr);
    /**
     * @brief Write the given chunk header to memory
     * @param chunkSimple RzHeapChunkSimple pointer of the chunk to be written
     * @return true if the write succeeded else false
     */
    bool writeHeapChunk(RzHeapChunkSimple *chunkSimple);
    int getArchBits();
    void startDebug();
    void startEmulation();
    /**
     * @brief attach to a remote debugger
     * @param uri remote debugger uri
     * @note attachedRemote(bool) signals the result
     */
    void attachRemote(const QString &uri);
    void attachDebug(int pid);
    void stopDebug();
    void suspendDebug();
    void syncAndSeekProgramCounter();
    void continueDebug();
    void continueBackDebug();
    void continueUntilCall();
    void continueUntilSyscall();
    void continueUntilDebug(ut64 offset);
    void stepDebug();
    void stepOverDebug();
    void stepOutDebug();
    void stepBackDebug();

    void startTraceSession();
    void stopTraceSession();

    void addBreakpoint(const BreakpointDescription &config);
    void updateBreakpoint(int index, const BreakpointDescription &config);
    void toggleBreakpoint(RVA addr);
    void delBreakpoint(RVA addr);
    void delAllBreakpoints();
    void enableBreakpoint(RVA addr);
    void disableBreakpoint(RVA addr);
    /**
     * @brief Enable or disable breakpoint tracing.
     * @param index - breakpoint index to modify
     * @param enabled - true if tracing should be enabled
     */
    void setBreakpointTrace(int index, bool enabled);
    int breakpointIndexAt(RVA addr);
    BreakpointDescription getBreakpointAt(RVA addr);

    bool isBreakpoint(const QList<RVA> &breakpoints, RVA addr);
    QList<RVA> getBreakpointsAddresses();

    /**
     * @brief Get all breakpoinst that are belong to a functions at this address
     */
    QList<RVA> getBreakpointsInFunction(RVA funcAddr);
    QString getActiveDebugPlugin();
    QStringList getDebugPlugins();
    void setDebugPlugin(QString plugin);
    bool isDebugTaskInProgress();
    /**
     * @brief Check if we can use output/input redirection with the currently debugged process
     */
    bool isRedirectableDebugee();
    bool currentlyDebugging = false;
    bool currentlyEmulating = false;
    bool currentlyTracing = false;
    bool currentlyRemoteDebugging = false;
    int currentlyAttachedToPID = -1;
    QString currentlyOpenFile;

    /* Decompilers */
    QList<Decompiler *> getDecompilers();
    Decompiler *getDecompilerById(const QString &id);

    /**
     * Register a new decompiler
     *
     * The decompiler must have a unique id, otherwise this method will fail.
     * The decompiler's parent will be set to this CutterCore instance, so it will automatically be
     * freed later.
     *
     * @return whether the decompiler was registered successfully
     */
    bool registerDecompiler(Decompiler *decompiler);

    RVA getOffsetJump(RVA addr);
    CutterJson getSignatureInfo();
    bool existsFileInfo();
    void setGraphEmpty(bool empty);
    bool isGraphEmpty();

    void getRegs();
    QList<QString> regs;
    void setSettings();

    void loadPDB(const QString &file);

    QByteArray ioRead(RVA addr, int len);

    QList<RVA> getSeekHistory();

    /* Plugins */
    QStringList getAsmPluginNames();
    QStringList getAnalysisPluginNames();

    /* Widgets */
    QList<RzBinPluginDescription> getBinPluginDescriptions(bool bin = true, bool xtr = true);
    QList<RzIOPluginDescription> getRIOPluginDescriptions();
    QList<RzCorePluginDescription> getRCorePluginDescriptions();
    QList<RzAsmPluginDescription> getRAsmPluginDescriptions();
    QList<FunctionDescription> getAllFunctions();
    QList<ImportDescription> getAllImports();
    QList<ExportDescription> getAllExports();
    QList<SymbolDescription> getAllSymbols();
    QList<HeaderDescription> getAllHeaders();
    QList<GlobalDescription> getAllGlobals();
    QList<FlirtDescription> getSignaturesDB();
    QList<CommentDescription> getAllComments(const QString &filterType);
    QList<RelocDescription> getAllRelocs();
    QList<StringDescription> getAllStrings();
    QList<FlagspaceDescription> getAllFlagspaces();
    QList<FlagDescription> getAllFlags(QString flagspace = QString());
    QList<SectionDescription> getAllSections();
    QList<SegmentDescription> getAllSegments();
    QList<EntrypointDescription> getAllEntrypoint();
    QList<BinClassDescription> getAllClassesFromBin();
    QList<BinClassDescription> getAllClassesFromFlags();
    QList<ResourcesDescription> getAllResources();
    QList<VTableDescription> getAllVTables();

    /**
     * @return all loaded types
     */
    QList<TypeDescription> getAllTypes();

    /**
     * @return all loaded primitive types
     */
    QList<TypeDescription> getAllPrimitiveTypes();

    /**
     * @return all loaded unions
     */
    QList<TypeDescription> getAllUnions();

    /**
     * @return all loaded structs
     */
    QList<TypeDescription> getAllStructs();

    /**
     * @return all loaded enums
     */
    QList<TypeDescription> getAllEnums();

    /**
     * @return all loaded typedefs
     */
    QList<TypeDescription> getAllTypedefs();

    /**
     * @brief Fetching the C representation of a given Type
     * @param name - the name or the type of the given Type
     * @return The type decleration as C output
     */
    QString getTypeAsC(QString name);

    /**
     * @brief Checks if the given address is mapped to a region
     * @param addr The address to be checked
     * @return true if addr is mapped, false otherwise
     */
    bool isAddressMapped(RVA addr);

    QList<MemoryMapDescription> getMemoryMap();
    QList<SearchDescription> getAllSearch(QString searchFor, SearchKind kind, QString in);
    QList<BreakpointDescription> getBreakpoints();
    /**
     * @brief Get list of processes attachable by debugger
     *
     * @param pid 0 - all processes, -1 - currently debugged process
     * @return QList<ProcessDescription>
     */
    QList<ProcessDescription> getProcesses(int pid = 0);
    /**
     * @brief Get the right RzReg object based on the cutter state (debugging vs emulating)
     */
    RzReg *getReg();
    /**
     * @brief returns a list of reg values and their telescoped references
     * @param depth telescoping depth
     */
    QList<RegisterRef> getRegisterRefs(int depth = 6);
    QVector<RegisterRefValueDescription> getRegisterRefValues();
    QList<VariableDescription> getVariables(RVA at);
    /**
     * @brief Fetches all the writes or reads to the specified local variable 'variableName'
     * in the function in which the specified offset is a part of.
     * @param variableName Name of the local variable.
     * @param findWrites If this is true, then locations at which modification happen to the
     * specified local variable is fetched. Else, the locations at which the local is variable is
     * read is fetched.
     * @param offset An offset in the function in which the specified local variable exist.
     * @return A list of XrefDescriptions that contains details of all the writes or reads that
     * happen to the variable 'variableName'.
     */
    QList<XrefDescription> getXRefsForVariable(QString variableName, bool findWrites, RVA offset);
    QList<XrefDescription> getXRefs(RVA addr, bool to, bool whole_function,
                                    const QString &filterType = QString());

    void handleREvent(int type, void *data);

    /* Signals related */
    void triggerVarsChanged();
    void triggerFunctionRenamed(const RVA offset, const QString &newName);
    void triggerRefreshAll();
    void triggerAsmOptionsChanged();
    void triggerGraphOptionsChanged();

    void message(const QString &msg, bool debug = false);

    QStringList getSectionList();

    RzCoreLocked lock();
    CUTTER_DEPRECATED("Use CutterCore::lock instead")
    RzCoreLocked core();

    static QString ansiEscapeToHtml(const QString &text);
    BasicBlockHighlighter *getBBHighlighter();
    BasicInstructionHighlighter *getBIHighlighter();

    /**
     * @brief Enable or dsiable Cache mode. Cache mode is used to imagine writing to the opened file
     * without committing the changes to the disk.
     * @param enabled
     */
    void setIOCache(bool enabled);

    /**
     * @brief Check if Cache mode is enabled.
     * @return true if Cache is enabled, otherwise return false.
     */
    bool isIOCacheEnabled() const;

    /**
     * @brief Commit write cache to the file on disk.
     */
    void commitWriteCache();
    /**
     * @brief Reset write cache.
     */
    void resetWriteCache();

    /**
     * @brief Enable or disable Write mode. When the file is opened in write mode, any changes to it
     * will be immediately committed to the file on disk, thus modify the file. This function wrap
     * Rizin function which re-open the file with the desired permissions.
     * @param enabled
     */
    void setWriteMode(bool enabled);
    /**
     * @brief Check if the file is opened in write mode.
     * @return true if write mode is enabled, otherwise return false.
     */
    bool isWriteModeEnabled();

    /**
     * @brief   Returns the textual version of global or specific graph.
     * @param   type     Graph type, example RZ_CORE_GRAPH_TYPE_FUNCALL or RZ_CORE_GRAPH_TYPE_IMPORT
     * @param   format   Graph format, example RZ_CORE_GRAPH_FORMAT_DOT or RZ_CORE_GRAPH_FORMAT_GML
     * @param   address  The object address (if global set it to RVA_INVALID)
     * @return  The textual graph string.
     */
    char *getTextualGraphAt(RzCoreGraphType type, RzCoreGraphFormat format, RVA address);

    /**
     * @brief   Writes a graphviz graph to a file.
     * @param   path     The file output path
     * @param   format   The output format (see graph.gv.format)
     * @param   type     The graph type, example RZ_CORE_GRAPH_TYPE_FUNCALL or
     * RZ_CORE_GRAPH_TYPE_IMPORT
     * @param   address  The object address (if global set it to RVA_INVALID)
     */
    void writeGraphvizGraphToFile(QString path, QString format, RzCoreGraphType type, RVA address);

signals:
    void refreshAll();

    void functionRenamed(const RVA offset, const QString &new_name);
    void varsChanged();
    void globalVarsChanged();
    void functionsChanged();
    void flagsChanged();
    void commentsChanged(RVA addr);
    void registersChanged();
    void instructionChanged(RVA offset);
    void breakpointsChanged(RVA offset);
    void refreshCodeViews();
    void stackChanged();
    /**
     * @brief update all the widgets that are affected by rebasing in debug mode
     */
    void codeRebased();

    void switchedThread();
    void switchedProcess();

    void classNew(const QString &cls);
    void classDeleted(const QString &cls);
    void classRenamed(const QString &oldName, const QString &newName);
    void classAttrsChanged(const QString &cls);

    /**
     * @brief end of current debug event received
     */
    void debugProcessFinished(int pid);

    void attachedRemote(bool successfully);

    void ioCacheChanged(bool newval);
    void writeModeChanged(bool newval);
    void ioModeChanged();

    /**
     * emitted when debugTask started or finished running
     */
    void debugTaskStateChanged();

    /**
     * emitted when config regarding disassembly display changes
     */
    void asmOptionsChanged();

    /**
     * emitted when config regarding graph display changes
     */
    void graphOptionsChanged();

    /**
     * @brief seekChanged is emitted each time Rizin's seek value is modified
     * @param offset
     * @param historyType
     */
    void seekChanged(RVA offset, SeekHistoryType type = SeekHistoryType::New);

    void toggleDebugView();

    void newMessage(const QString &msg);
    void newDebugMessage(const QString &msg);

    void showMemoryWidgetRequested();

private:
    /**
     * Internal reference to the RzCore.
     * NEVER use this directly! Always use the CORE_LOCK(); macro and access it like core->...
     */
    RzCore *core_ = nullptr;
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    QMutex coreMutex;
#else
    QRecursiveMutex coreMutex;
#endif
    int coreLockDepth = 0;
    void *coreBed = nullptr;

    AsyncTaskManager *asyncTaskManager;
    RVA offsetPriorDebugging = RVA_INVALID;
    QErrorMessage msgBox;

    QList<Decompiler *> decompilers;

    bool emptyGraph = false;
    BasicBlockHighlighter *bbHighlighter;
    bool iocache = false;
    BasicInstructionHighlighter biHighlighter;

    QSharedPointer<RizinTask> debugTask;
    RizinTaskDialog *debugTaskDialog;

    QVector<QString> getCutterRCFilePaths() const;
    QList<TypeDescription> getBaseType(RzBaseTypeKind kind, const char *category);
    QList<SearchDescription> getAllSearchCommand(QString searchFor, SearchKind kind, QString in);
};

class CUTTER_EXPORT RzCoreLocked
{
    CutterCore *const core;

public:
    explicit RzCoreLocked(CutterCore *core);
    RzCoreLocked(const RzCoreLocked &) = delete;
    RzCoreLocked &operator=(const RzCoreLocked &) = delete;
    RzCoreLocked(RzCoreLocked &&);
    ~RzCoreLocked();
    operator RzCore *() &;
    RzCore *operator->() &;
    // Reduce chance of following misuse of Core()->lock()
    // rizinStruct* foo = rizin_func(Core()->lock()->something, arg);
    operator RzCore *() && = delete;
    RzCore *operator->() && = delete;
};

#endif // CUTTER_H
