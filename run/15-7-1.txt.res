Step 1/5: Init Sampler
Finished
State type: (Int[0,0]*List[6](Int[0,2]))
Env type: [List[6](List[6]((Int[0,2]*Int[0,2])))]



Step 2/5: Synthesize PreOrder for Plan
Finished 0.040967
Cmp:[<=:let p = zip(Param@0,tail(Param@0)) in let f = (\ x@(Int*Int) -> access(access(Param@1,x.1),x.2)) in let f2 = (\ y@(Int*Int) -> y.2) in sum(map(f2,map(f,p)))]



Step 3/5: Rewrite F via PreOrders
Finished 1.431511
Cared Funtions
	let p = zip(Param@0,tail(Param@0)) in let f = (\ x@(Int*Int) -> access(access(Param@1,x.1),x.2)) in let f2 = (\ y@(Int*Int) -> y.2) in sum(map(f2,map(f,p)))
	fold((\ tmp0@Int tmp1@Int -> tmp0),0,Param@0)
State: (Int[0,0]*List[6](Int[0,2]))	 Plan: (Int*Int)
T: if (==(size(Param@1),0)) then collect(1,Param@0) else foreach i in ..(0,+(size(Param@2),-1)), let e = access(access(Param@2,Param@0),i) in if (==(e.1,head(Param@1))) then collect(2,'(Param@0,'(i,tail(Param@1))));
F:
	collect(0,'(0,Param@0))
	collect(0,'(+(Param@1.1,access(access(Param@2,Param@0),Param@1.2).2),Param@0))



Step 4/5: Synthesize Eq Relation for States
Finished 1.687011
Eq:[Param@0,fold((\ tmp0@Int tmp1@Int -> +(1,tmp1)),0,Param@1)]



Step 5/5: Rewrite T via Eq Relation
Finished 0.436366
Cared Funtions
	Param@0
	fold((\ tmp0@Int tmp1@Int -> +(1,tmp1)),0,Param@1)
State: (Int[0,0]*Int)	 Plan: (Int*Int)
T: if (==(Param@1,0)) then collect(1,Param@0) else foreach i in ..(0,+(size(Param@2),-1)), if (==(access(access(Param@2,Param@0),i).1,access(Param@3.2,neg(Param@1)))) then collect(2,'(Param@0,'(i,+(Param@1,neg(1)))));
F:
	collect(0,'(0,Param@0))
	collect(0,'(+(Param@1.1,access(access(Param@2,Param@0),Param@1.2).2),Param@0))
