Step 1/5: Init Sampler
Finished
State type: List[13](Int[0,1000])
Env type: []



Step 2/5: Synthesize PreOrder for Plan
Finished 1.479185
Cmp:[=:fold((\ tmp0@(Int*Int) tmp1@Int -> tmp0.2),0,Param@0.2),=:fold((\ tmp0@(Int*Int) tmp1@Int -> tmp0.1),0,Param@0.1),<=:let dis = (\ x@(Int*Int) y@(Int*Int) -> +(sqr(-(x.1,y.1)),sqr(-(x.2,y.2)))) in let dis2 = (\ z@((Int*Int)*(Int*Int)) -> dis(z.1,z.2)) in let lp = zip(Param@0.1,tail(Param@0.1)) in let rp = zip(Param@0.2,tail(Param@0.2)) in -(-(neg(sum(map(dis2,lp))),sum(map(dis2,rp))),dis(head(Param@0.1),head(Param@0.2)))]



Step 3/5: Rewrite F via PreOrders
Failed 120.029589



Step 4/5: Synthesize Eq Relation for States
Finished 3.989766
Eq:[fold((\ tmp0@Int tmp1@Int -> +(1,tmp1)),0,Param@0)]



Step 5/5: Rewrite T via Eq Relation
Finished 7.979227
Cared Funtions
	fold((\ tmp0@Int tmp1@Int -> +(1,tmp1)),0,Param@0)
State: Int	 Plan: (List((Int*Int))*List((Int*Int)))
T: if (==(Param@0,1)) then collect(2,access(Param@1,neg(1))) else collect(1,'('(Param@0,access(Param@1,neg(Param@0))),+(Param@0,neg(1))))
F:
	let l_list = Param@1.1 in let r_list = Param@1.2 in collect(0,'(cons(Param@0,l_list),r_list));collect(0,'(l_list,cons(Param@0,r_list)));
	collect(0,'(cons('(1,Param@0),nil('(1,1))),cons('(1,Param@0),nil('(1,1)))))
