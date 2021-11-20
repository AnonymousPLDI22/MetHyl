//
// Created by pro on 2021/10/25.
//

#include "hylodp.h"
#include "glog/logging.h"
#include "plan_order_solver.h"
#include "incre_rewriter.h"
#include "state_mapping_solver.h"
#include "relation_rewriter.h"

HyloDPRes hylodp::synthesisDP(Task *task, const std::string& oup_file) {
    FILE* oup = nullptr;
    if (!oup_file.empty()) oup = fopen(oup_file.c_str(), "w");
    auto print_info = [=](const std::string& s) {
        LOG(INFO) << s;
        if (oup) fprintf(oup, "%s\n", s.c_str());
    };
    print_info("Step 1/5: Init Sampler");
    auto* sampler = new InputSampler(task);
    print_info("Finished");
    sampler->printType(oup);
    TimeRecorder rec;
    print_info("\n\n");

    auto* p_solver = new PlanOrderSolver(sampler, config::KTimeOut);
    print_info("Step 2/5: Synthesize PreOrder for Plan");
    PreOrder* order = new EmptyPreOder();
    try {
        order = p_solver->synthesisPreOrder();
        print_info("Finished " + std::to_string(rec.getTime()));
        print_info(order->toString());
        sampler->e->plan_order = order; sampler->e->verifyViaSample();
    } catch (TimeOutError& e) {
        print_info("Failed " + std::to_string(rec.getTime()));
    }
    print_info("\n\n");

    print_info("Step 3/5: Rewrite F via PreOrders");
    auto* task2 = task;
    try {
        auto* rewriter = new PlanRewriter(sampler, order, config::KTimeOut);
        rewriter->rewrite();
        print_info("Finished " + std::to_string(rec.getTime()));
        print_info("Cared Funtions");
        for (auto* f: rewriter->cared_values) {
            print_info("\t" + f->toString());
        }
        task2 = rewriter->getNewTask();
        task2->print(oup);
        auto* executor2 = rewriter->getNewExecutor();
        executor2->verifyViaSample();
    } catch (RewriterException& e){
        print_info("Failed " + std::to_string(rec.getTime()));
    } catch (TimeOutError& e) {
        print_info("Failed " + std::to_string(rec.getTime()));
    }
    print_info("\n\n");

    print_info("Step 4/5: Synthesize Eq Relation for States");
    auto* sampler2 = new InputSampler(task2);
    KeyEqRelation* state_relation = nullptr;
    try {
        auto *state_solver = new StateMappingSolver(sampler2, config::KTimeOut);
        state_relation = state_solver->synthesisEqRelation();
        print_info("Finished " + std::to_string(rec.getTime()));
        print_info(state_relation->toString());
        sampler2->e->state_relation = state_relation; sampler2->e->verifyViaSample();
    } catch (TimeOutError& e) {
        print_info("Failed " + std::to_string(rec.getTime()));
    }
    print_info("\n\n");

    print_info( "Step 5/5: Rewrite T via Eq Relation");
    try {
        auto* relation_rewriter = new RelationRewriter(sampler2, state_relation);
        relation_rewriter->rewrite();
        print_info("Finished " + std::to_string(rec.getTime()));
        print_info("Cared Funtions");
        for (auto* f: relation_rewriter->cared_functions) {
            print_info("\t" + f->toString());
        }
        auto* task3 = relation_rewriter->getNewTask();
        task3->print(oup);
        auto* executor3 = relation_rewriter->getNewExecutor(); executor3->verifyViaSample();
    } catch (RewriterException& e){
        print_info("Failed " + std::to_string(rec.getTime()));
    } catch (TimeOutError& e) {
        print_info("Failed " + std::to_string(rec.getTime()));
    }
}