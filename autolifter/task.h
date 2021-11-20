//
// Created by pro on 2020/11/20.
//

#ifndef CPP_TASK_H
#define CPP_TASK_H

#include <unordered_map>
#include "grammar.h"
#include "program.h"
#include "example_space.h"
#include "polygen/polygen_config.h"

namespace autolifter {
    class TaskCache;

    class SepCacheItem {
    public:
        Program *f;
        TaskCache *tc;
        DataStorage result;

        DataList getSeparateOup(int k);

        SepCacheItem(Program *_f, TaskCache *_tc, DataStorage &&_result);

        ~SepCacheItem() = default;
    };

    class MergedCacheItem {
    public:
        TaskCache *tc;
        Program *f;
        DataList result;

        Data getMergedOup(int id, bool is_rerun = false);

        MergedCacheItem(Program *_f, TaskCache *_tc, DataList &&_result);

        ~MergedCacheItem() = default;
    };

    struct AutoLifterConfig {
        polygen::PolyGenConfig poly_config;
        int KInitExampleNum = 100;
        int KTermSolverT = 1;
        int KExtraTermRoundNum = 0;
        int KVerifyEqSizeMax = 1;
    };

    typedef std::pair<MergedCacheItem *, SepCacheItem *> CacheItem;

    class TaskCache {
    public:
        Program *m;
        Type *F;
        ExampleSpace *example_space;
        std::vector<SepCacheItem *> sep_item_list;
        std::vector<MergedCacheItem *> merged_item_list;
        std::unordered_map<std::string, int> name_map;

        TaskCache(Program *_m, Type *_Func, ExampleSpace *_example_space);

        DataList executeSeparate(Program *program, int id);

        Data executeMerged(Program *program, int id);

        CacheItem getCacheItem(Program *program);

        CacheItem insertCache(Program *program, DataList &&merged_res, DataStorage &&sep_res);

        ~TaskCache();
    };

    class Task {
    public:
        TaskCache *cache;
        Grammar *g;
        Program *p = nullptr;
        MergedCacheItem *p_mer = nullptr;
        SepCacheItem *p_sep = nullptr;
        AutoLifterConfig config;

        Task(Program* _m, Type* _F, ExampleSpace* example_space, Grammar* _g, const AutoLifterConfig& c);

        ~Task() { delete cache; }

        std::pair<Data, DataList> getPrecondition(int id);

        std::pair<Data, DataList> executeProgram(Program *program, int id);

        DataList executeSeparate(Program *program, int id);

        bool evaluate(Program *program, std::pair<int, int> id_pair);

        bool isCached(Program *program);

        void insertCache(Program *program, DataList &&merged_res, DataStorage &&separate_res);

        PointExample buildCExample(const ProgramList &lifting_list, int pos);

        void setTarget(Program *_p);

        bool checkFEqual(Program *p1, Program *p2);
    };

    Task* buildTask(Program* _m, Type* _F, ExampleSpace* example_space, Grammar* _g);
}


#endif //CPP_TASK_H
