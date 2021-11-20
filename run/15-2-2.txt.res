Step 1/5: Init Sampler
Finished
State type: List[10](Int[0,4])
Env type: []



Step 2/5: Synthesize PreOrder for Plan
Finished 0.023288
Cmp:[<=:size(Param@0)]



Step 3/5: Rewrite F via PreOrders
Finished 1.857030
Cared Funtions
	size(Param@0)
State: List[10](Int[0,4])	 Plan: Int
T: let l = size(Param@0) in if (==(l,0)) then collect(4,empty);if (==(l,1)) then collect(3,head(Param@0));if (>(l,1)) then collect(2,tail(Param@0));collect(2,reverse(tail(reverse(Param@0))));collect(1,'('(head(Param@0),access(Param@0,-1)),reverse(tail(reverse(tail(Param@0))))));;
F:
	if (==(Param@0.1,Param@0.2)) then collect(0,+(1,+(1,Param@1)))
	collect(0,Param@0)
	collect(0,1)
	collect(0,0)



Step 4/5: Synthesize Eq Relation for States
Finished 1.115237
Eq:[lmatch(Param@0,Param@1),fold((\ tmp0@Int tmp1@Int -> +(1,tmp1)),0,Param@0)]



Step 5/5: Rewrite T via Eq Relation
Finished 2.564016
Cared Funtions
	lmatch(Param@0,Param@1)
	fold((\ tmp0@Int tmp1@Int -> +(1,tmp1)),0,Param@0)
State: (Int*Int)	 Plan: Int
T: if (==(Param@1,0)) then collect(4,empty);if (==(Param@1,1)) then collect(3,access(Param@2,Param@0));if (>(Param@1,1)) then collect(2,'(lmove(Param@2,Param@0,1,Param@1),+(Param@1,neg(1))));collect(2,'(lmove(Param@2,Param@0,0,+(Param@1,neg(1))),+(Param@1,neg(1))));collect(1,'('(access(Param@2,Param@0),access(Param@2,+(-1,+(Param@0,Param@1)))),'(lmove(Param@2,Param@0,1,+(Param@1,neg(1))),+(Param@1,neg(+(1,1))))));;
F:
	if (==(Param@0.1,Param@0.2)) then collect(0,+(1,+(1,Param@1)))
	collect(0,Param@0)
	collect(0,1)
	collect(0,0)
