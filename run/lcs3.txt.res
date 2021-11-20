Step 1/5: Init Sampler
Finished
State type: (List[7](Int[0,7])*List[7](Int[0,7]))
Env type: []



Step 2/5: Synthesize PreOrder for Plan
Finished 0.288293
Cmp:[<=:let f = (\ x@(Int*Int) y@Int -> ite(&&(==(x.1,x.2),not(==(y,-1))),+(y,1),-1)) in fold(f,0,Param@0)]



Step 3/5: Rewrite F via PreOrders
Finished 14.355300
Cared Funtions
	let f = (\ x@(Int*Int) y@Int -> ite(&&(==(x.1,x.2),not(==(y,-1))),+(y,1),-1)) in fold(f,0,Param@0)
State: (List[1000](Int[0,1000])*List[1000](Int[0,1000]))	 Plan: Int
T: if (&&(==(size(Param@0),0),==(size(Param@1),0))) then collect(3,empty);if (>(size(Param@0),0)) then collect(1,'(tail(Param@0),Param@1));if (>(size(Param@1),0)) then collect(1,'(Param@0,tail(Param@1)));if (&&(>(size(Param@0),0),>(size(Param@1),0))) then collect(2,'('(head(Param@0),head(Param@1)),'(tail(Param@0),tail(Param@1))));
F:
	collect(0,Param@0)
	collect(0,ite(&&(==(Param@0.1,Param@0.2),<=(1,+(Param@1,1))),+(Param@1,1),-1))
	collect(0,0)



Step 4/5: Synthesize Eq Relation for States
Finished 3.443544
Eq:[fold((\ tmp0@Int tmp1@Int -> +(1,tmp1)),0,Param@0),fold((\ tmp0@Int tmp1@Int -> +(1,tmp1)),0,Param@1)]



Step 5/5: Rewrite T via Eq Relation
Finished 6.251750
Cared Funtions
	fold((\ tmp0@Int tmp1@Int -> +(1,tmp1)),0,Param@0)
	fold((\ tmp0@Int tmp1@Int -> +(1,tmp1)),0,Param@1)
State: (Int*Int)	 Plan: Int
T: if (&&(==(Param@0,0),==(Param@1,0))) then collect(3,empty);if (>(Param@0,0)) then collect(1,'(+(Param@0,neg(1)),Param@1));if (>(Param@1,0)) then collect(1,'(Param@0,+(Param@1,neg(1))));if (&&(>(Param@0,0),>(Param@1,0))) then collect(2,'('(access(Param@2.1,neg(Param@0)),access(Param@2.2,neg(Param@1))),'(+(Param@0,neg(1)),+(Param@1,neg(1)))));
F:
	collect(0,Param@0)
	collect(0,ite(&&(==(Param@0.1,Param@0.2),<=(1,+(Param@1,1))),+(Param@1,1),-1))
	collect(0,0)
