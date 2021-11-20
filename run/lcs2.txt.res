Step 1/5: Init Sampler
Finished
State type: (List[8](Int[0,1000])*List[8](Int[0,1000]))
Env type: []



Step 2/5: Synthesize PreOrder for Plan
Finished 0.107362
Cmp:[<=:size(Param@0)]



Step 3/5: Rewrite F via PreOrders
Finished 1.579741
Cared Funtions
	size(Param@0)
State: (List[1000](Int[0,1000])*List[1000](Int[0,1000]))	 Plan: Int
T: if (&&(==(size(Param@0),0),==(size(Param@1),0))) then collect(3,empty);if (>(size(Param@0),0)) then collect(1,'(tail(Param@0),Param@1));if (>(size(Param@1),0)) then collect(1,'(Param@0,tail(Param@1)));if (&&(>(size(Param@0),0),>(size(Param@1),0))) then collect(2,'('(head(Param@0),head(Param@1)),'(tail(Param@0),tail(Param@1))));
F:
	collect(0,Param@0)
	if (==(Param@0.1,Param@0.2)) then collect(0,+(1,Param@1))
	collect(0,0)



Step 4/5: Synthesize Eq Relation for States
Finished 2.767789
Eq:[fold((\ tmp0@Int tmp1@Int -> +(1,tmp1)),0,Param@1),fold((\ tmp0@Int tmp1@Int -> +(1,tmp1)),0,Param@0)]



Step 5/5: Rewrite T via Eq Relation
Finished 1.445068
Cared Funtions
	fold((\ tmp0@Int tmp1@Int -> +(1,tmp1)),0,Param@1)
	fold((\ tmp0@Int tmp1@Int -> +(1,tmp1)),0,Param@0)
State: (Int*Int)	 Plan: Int
T: if (&&(==(Param@1,0),==(Param@0,0))) then collect(3,empty);if (>(Param@1,0)) then collect(1,'(Param@0,+(Param@1,neg(1))));if (>(Param@0,0)) then collect(1,'(+(Param@0,neg(1)),Param@1));if (&&(>(Param@1,0),>(Param@0,0))) then collect(2,'('(access(Param@2.1,neg(Param@1)),access(Param@2.2,neg(Param@0))),'(+(Param@0,neg(1)),+(Param@1,neg(1)))));
F:
	collect(0,Param@0)
	if (==(Param@0.1,Param@0.2)) then collect(0,+(1,Param@1))
	collect(0,0)
