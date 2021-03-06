#include "sqlstats.h"

#ifdef _MSC_VER
#define NOMINMAX
#include <windows.h>
#include <mysql.h>
#else
#include <mysql/mysql.h>
#endif

using namespace CMSat;


class MySQLStats: public SQLStats
{
public:
    MySQLStats();
	virtual ~MySQLStats();

    virtual void restart(
        const PropStats& thisPropStats
        , const Searcher::Stats& thisStats
        , const VariableVariance& varVarStats
        , const Solver* solver
        , const Searcher* searcher
    );

    virtual void clauseSizeDistrib(
        uint64_t sumConflicts
        , const vector<uint32_t>& sizes
    );

    virtual void clauseGlueDistrib(
        uint64_t sumConflicts
        , const vector<uint32_t>& glues
    );

    virtual void clauseSizeGlueScatter(
        uint64_t sumConflicts
        , boost::multi_array<uint32_t, 2>& sizeAndGlue
    );

    virtual void varDataDump(
        const Solver* solver
        , const Searcher* search
        , const vector<Var>& varsToDump
        , const vector<VarData>& varData
    );

    virtual void reduceDB(
        const ClauseUsageStats& irredStats
        , const ClauseUsageStats& redStats
        , const CleaningStats& clean
        , const Solver* solver
    );

    virtual void setup(const Solver* solver);

private:

    struct StmtClsDistrib {
        StmtClsDistrib() :
            stmt(NULL)
        {}

        vector<MYSQL_BIND>  bind;
        MYSQL_STMT  *stmt;

        //Variables
        uint64_t sumConflicts;
        vector<uint64_t> value;
        vector<uint64_t> num;
    };

    void connectServer(const Solver* solver);
    void getID(const Solver* solver);
    bool tryIDInSQL(const Solver* solver);

    void addFiles(const Solver* solver);
    void addStartupData(const Solver* solver);
    void initRestartSTMT(uint64_t verbosity);
    void initClauseDistribSTMT(
        const Solver* solver
        , StmtClsDistrib& stmt
        , const string& tableName
        , const string& valueName
        , const size_t numInserts
    );
    void initSizeGlueScatterSTMT(
        const Solver* solver
        , const size_t numInserts
    );

    void writeQuestionMarks(size_t num, std::stringstream& ss);

    template<typename T>
    void bindTo(
        T& t
        , uint64_t& data
    ) {
        t.bind[bindAt].buffer_type= MYSQL_TYPE_LONG;
        t.bind[bindAt].buffer= (char *)&data;
        t.bind[bindAt].is_null= 0;
        t.bind[bindAt].length= 0;

        bindAt++;
    }

    template<typename T>
    void bindTo(
        T& t
        , double& data
    ) {
        t.bind[bindAt].buffer_type= MYSQL_TYPE_DOUBLE;
        t.bind[bindAt].buffer= (char *)&data;
        t.bind[bindAt].is_null= 0;
        t.bind[bindAt].length= 0;

        bindAt++;
    }

    struct StmtReduceDB {
        StmtReduceDB() :
            stmt(NULL)
        {};

        MYSQL_BIND  bind[10 + 13*3];
        MYSQL_STMT  *stmt;

        //Position
        uint64_t numSimplify;
        uint64_t sumRestarts;
        uint64_t sumConflicts;
        double cpuTime;
        uint64_t reduceDBs;

        //Actual data -- irred
        uint64_t irredClsVisited;
        uint64_t irredLitsVisited;

        //Actual data -- red
        uint64_t redClsVisited;
        uint64_t redLitsVisited;

        //Cleaning stats
        CleaningStats clean;
    };
    StmtReduceDB stmtReduceDB;
    void initReduceDBSTMT(uint64_t verbosity);

    struct StmtVar {
        StmtVar() :
            stmt(NULL)
        {};

        uint64_t varInitID;
        vector<MYSQL_BIND>  bind;
        MYSQL_STMT  *stmt;

        struct Data {
            uint64_t var;

            //Overall stats
            uint64_t posPolarSet;
            uint64_t negPolarSet;
            uint64_t flippedPolarity;
            uint64_t posDecided;
            uint64_t negDecided;

            //Dec level history stats
            double decLevelAvg;
            double decLevelSD;
            uint64_t decLevelMin;
            uint64_t decLevelMax;

            //Trail level history stats
            double trailLevelAvg;
            double trailLevelSD;
            uint64_t trailLevelMin;
            uint64_t trailLevelMax;
        };
        vector<Data> data;
    };
    StmtVar stmtVarBulk;
    StmtVar stmtVarSingle;
    void initVarSTMT(
        const Solver* solver
        , MySQLStats::StmtVar& stmtVar
        , uint64_t numInserts
    );

    size_t bindAt;
    struct StmtRst {
        StmtRst() :
            stmt(NULL)
        {};

        MYSQL_BIND  bind[81+1]; //+1 == runID
        MYSQL_STMT  *stmt;

        //Position
        uint64_t numSimplify;
        uint64_t sumRestarts;
        uint64_t sumConflicts;
        double cpuTime;

        //Clause stats
        uint64_t numIrredBins;
        uint64_t numIrredTris;
        uint64_t numIrredLongs;
        uint64_t numIrredLits;
        uint64_t numRedBins;
        uint64_t numRedTris;
        uint64_t numRedLongs;
        uint64_t numRedLits;

        //Conflict stats
        double glueHist;
        double glueHistSD;
        uint64_t glueHistMin;
        uint64_t glueHistMax;

        double conflSizeHist;
        double conflSizeHistSD;
        uint64_t conflSizeHistMin;
        uint64_t conflSizeHistMax;

        double numResolutionsHist;
        double numResolutionsHistSD;
        uint64_t numResolutionsHistMin;
        uint64_t numResolutionsHistMax;

        double conflictAfterConflict;

        //Search stats
        double branchDepthHist;
        double branchDepthHistSD;
        uint64_t branchDepthHistMin;
        uint64_t branchDepthHistMax;

        double branchDepthDeltaHist;
        double branchDepthDeltaHistSD;
        uint64_t branchDepthDeltaHistMin;
        uint64_t branchDepthDeltaHistMax;

        double trailDepthHist;
        double trailDepthHistSD;
        uint64_t trailDepthHistMin;
        uint64_t trailDepthHistMax;

        double trailDepthDeltaHist;
        double trailDepthDeltaHistSD;
        uint64_t trailDepthDeltaHistMin;
        uint64_t trailDepthDeltaHistMax;

        double agilityHist;

        //Prop
        uint64_t propsBinIrred;
        uint64_t propsBinRed;
        uint64_t propsTriIrred;
        uint64_t propsTriRed;
        uint64_t propsLongIrred;
        uint64_t propsLongRed;

        //Confl
        uint64_t conflsBinIrred;
        uint64_t conflsBinRed;
        uint64_t conflsTriIrred;
        uint64_t conflsTriRed;
        uint64_t conflsLongIrred;
        uint64_t conflsLongRed;

        //Learnt
        uint64_t learntUnits;
        uint64_t learntBins;
        uint64_t learntTris;
        uint64_t learntLongs;

        //Misc
        double watchListSizeTraversed;
        double watchListSizeTraversedSD;
        uint64_t watchListSizeTraversedMin;
        uint64_t watchListSizeTraversedMax;

        double litPropagatedSomething;
        double litPropagatedSomethingSD;

        //Resolution stats
        ResolutionTypes<uint64_t> resolv;

        //Var stats
        uint64_t propagations;
        uint64_t decisions;
        VariableVariance varVarStats;
        uint64_t varFlipped;
        uint64_t varSetNeg;
        uint64_t varSetPos;
        uint64_t numFreeVars;
        uint64_t numReplacedVars;
        uint64_t numVarsElimed;
        uint64_t trailSize;
    };

    struct StmtSizeGlueScatter {
        StmtSizeGlueScatter() :
            stmt(NULL)
        {}
        vector<MYSQL_BIND>  bind;
        MYSQL_STMT  *stmt;

        //Variables
        uint64_t sumConflicts;
        vector<uint64_t> size;
        vector<uint64_t> glue;
        vector<uint64_t> num;
    };

    StmtRst stmtRst;
    StmtSizeGlueScatter stmtSizeGlueScatter;
    StmtClsDistrib stmtClsDistribSize;
    StmtClsDistrib stmtClsDistribGlue;

    MYSQL *serverConn;
};