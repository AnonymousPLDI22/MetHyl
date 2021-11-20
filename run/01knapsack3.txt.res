Step 1/5: Init Sampler
Finished
State type: List[10]((Int[0,10]*Int[0,1000]))
Env type: [Int[0,10]]



Step 2/5: Synthesize PreOrder for Plan
Finished 3.812632
Cmp:[<=:neg(fold((\ tmp0@(Int*Int) tmp1@Int -> +(tmp1,tmp0.1)),0,Param@0)),<=:let getw = (\ x@(Int*Int) -> x.2) in let getv = (\ x@(Int*Int) -> x.1) in ite(>(sum(map(getv,Param@0)),Param@1),-1,sum(map(getw,Param@0)))]



Step 3/5: Rewrite F via PreOrders
Finished 17.479416
Cared Funtions
	neg(fold((\ tmp0@(Int*Int) tmp1@Int -> +(tmp1,tmp0.1)),0,Param@0))
	let getw = (\ x@(Int*Int) -> x.2) in let getv = (\ x@(Int*Int) -> x.1) in ite(>(sum(map(getv,Param@0)),Param@1),-1,sum(map(getw,Param@0)))
State: List[10]((Int[0,10]*Int[0,1000]))	 Plan: (Int*Int)
T: if (==(size(Param@0),0)) then collect(1,empty) else let item = head(Param@0) in let remain_list = tail(Param@0) in collect(2,'(item,remain_list));
F:
	collect(0,'(0,0))
	collect(0,'(Param@1.1,Param@1.2));collect(0,'(+(Param@1.1,neg(Param@0.1)),ite(<(+(Param@2,Param@1.1),Param@0.1),-1,+(Param@1.2,Param@0.2))));



Step 4/5: Synthesize Eq Relation for States
Finished 0.353303
Eq:[fold((\ tmp0@(Int*Int) tmp1@Int -> +(1,tmp1)),0,Param@0)]



Step 5/5: Rewrite T via Eq Relation
Finished 2.142922
Cared Funtions
	fold((\ tmp0@(Int*Int) tmp1@Int -> +(1,tmp1)),0,Param@0)
State: Int	 Plan: (Int*Int)
T: if (==(Param@0,0)) then collect(1,empty) else collect(2,'('(access(Param@2,neg(Param@0)).1,access(Param@2,neg(Param@0)).2),+(Param@0,neg(1))));
F:
	collect(0,'(0,0))
	collect(0,'(Param@1.1,Param@1.2));collect(0,'(+(Param@1.1,neg(Param@0.1)),ite(<(+(Param@2,Param@1.1),Param@0.1),-1,+(Param@1.2,Param@0.2))));
