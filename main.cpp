#include <iostream>
#include "parse_util.h"
#include "config.h"
#include "solver/grammar.h"
#include "solver/enumerator.h"
#include "state_mapping_solver.h"
#include "autolifter/autolifter.h"
#include "executor.h"
#include "input_sampler.h"
#include "plan_order_solver.h"
#include "polygen/polygen.h"
#include "glog/logging.h"
#include "incre_rewriter.h"
#include "relation_rewriter.h"
#include <cassert>
#include "hylodp.h"

#define testParser(type, val, tag) \
    { \
        auto* res = parse::parse ## type(val, tag); \
        std::cout << res->toString() << std::endl; \
    } \

#define testTypeParser(type, val, tag) \
    { \
        auto* res = parse::parse ## type(val, tag); \
        std::cout << res->getName() << std::endl; \
    } \

void runTests() {
    testParser(Program, "f(x:Bool, y:Int, z:Int, w:Int) if (x) collect(1, y + z - - w);", false)
    testTypeParser(Type, "(Int * Int)", false);
    testParser(Program, "f(a:Int, b:Int, w:(Int + Int * (Int + Bool) + Bool)) foreach x in a .. b, collect(1, w);", false)
    testParser(Program, "f(a:Int) let x = \\(b:Int). (a + b) in x(a);", false)
}
void testParse(const std::string& path) {
    auto* task = parse::parseTask(path.c_str(), true);
    auto* e = new Executor(task);
    //auto* sampler = new InputSampler(task);
    for (auto& example: task->sample_example_list) {
        std::cout << e->execute(example.inp, example.env) << " " << example.oup.getInt() << std::endl;
    }
}

void testPolyGen() {
    auto f = [](const DataList& x) -> Data {
        int ans = x[0].getInt();
        for (int i = 1; i < x.size(); ++i) {
            ans = std::max(ans, x[i].getInt());
        }
        return ans;
    };
    std::vector<PointExample*> example_list;
    for (int i = 1; i <= 100; ++i) {
        DataList x;
        for (int j = 1; j <= 5; j++) x.push_back(rand() % 100);
        example_list.push_back(new PointExample(x, f(x)));
    }
    auto solver = new polygen::PolyGen(polygen::PolyGenConfig());
    auto* res = solver->synthesis(example_list);
    std::cout << res->toString() << std::endl;
}

void testSolve(const std::string& path, std::string oup) {
    auto* task = parse::parseTask(path.c_str(), true);
    task->print();
    LOG(INFO) << "Parse success" << std::endl;
    hylodp::synthesisDP(task, oup);
}

void testExtendedPolyGen() {
    auto example_gen = []() -> PointExample* {
        Data x1(rand() % 10), x2(rand() % 10);
        Data x(new ProdValue({x1, x2}));
        Data y = rand() % 100;
        DataList z_contents;
        int size = rand() % 5 + 1;
        for (int i = 1; i <= size; ++i) {
            Data a(rand() % 10), b(rand() % 10);
            z_contents.emplace_back(new ProdValue({a, b}));
        }
        Data z(new ListValue(z_contents));
        return new PointExample({x, y, z}, z_contents[0].accessProd(0).getInt() + x2.getInt());
    };
    std::vector<PointExample*> example_space;
    for (int i = 0; i < 10; ++i) {
        auto* example = example_gen();
        example_space.push_back(example);
        std::cout << example::pointExample2String(*example) << std::endl;
    }
    polygen::PolyGenConfig pc;
    pc.int_consts = {0};
    auto* solver = new polygen::PolyGen(pc);
    auto* res = solver->synthesis(example_space);
    std::cout << res->toString() << std::endl;
}

int main(int argc, const char* argv[])
{
    std::string task = "15-5";
    std::string file = "15-5-4.txt";
    std::string benchmark = config::KSourcePath + "/dataset/" + task + "/" + file;
    std::string oup_file = config::KSourcePath + "/run/" + file + ".res";
    // std::string benchmark = config::KSourcePath + "/dataset/01knapsack/01knapsack1.txt";
    //testTypeParser(LimitedType, "BTree[10](Int[1, 1000], Int[1, 1000])", false);
    //testParser(Program, config::KSourcePath + "/test.txt", true)
    //testParse(benchmark);
    //exit(0);
    testSolve(benchmark, oup_file);
    // testPolyGen();
}