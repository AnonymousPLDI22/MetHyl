Step 1/5: Init Sampler
Finished
State type: List[13](Int[0,1000])
Env type: []



Step 2/5: Synthesize PreOrder for Plan
Finished 0.045774
Cmp:[<=:size(Param@0)]



Step 3/5: Rewrite F via PreOrders
Finished 3.475077
Cared Funtions
	size(Param@0)
State: List[1000](Int[0,1000])	 Plan: Int
T: let l = size(Param@0) in if (==(l,0)) then collect(4,empty);if (==(l,1)) then collect(3,head(Param@0));if (>(l,1)) then collect(2,tail(Param@0));collect(2,reverse(tail(reverse(Param@0))));if (==(head(Param@0),head(reverse(Param@0)))) then collect(1,'(head(Param@0),tail(reverse(tail(reverse(Param@0))))));;
F:
	collect(0,+(1,+(1,Param@1)))
	collect(0,Param@0)
	collect(0,1)
	collect(0,0)



Step 4/5: Synthesize Eq Relation for States
Finished 1.932934
Eq:[lmatch(Param@0,Param@1),fold((\ tmp0@Int tmp1@Int -> +(1,tmp1)),0,Param@0)]



Step 5/5: Rewrite T via Eq Relation
Finished 3.247240
Cared Funtions
	lmatch(Param@0,Param@1)
	fold((\ tmp0@Int tmp1@Int -> +(1,tmp1)),0,Param@0)
State: (Int*Int)	 Plan: Int
T: if (==(Param@1,0)) then collect(4,empty);if (==(Param@1,1)) then collect(3,access(Param@2,Param@0));if (>(Param@1,1)) then collect(2,'(lmove(Param@2,Param@0,1,Param@1),+(Param@1,neg(1))));collect(2,'(lmove(Param@2,Param@0,0,+(Param@1,neg(1))),+(Param@1,neg(1))));if (==(access(Param@2,Param@0),access(Param@2,+(Param@0,+(Param@1,neg(1)))))) then collect(1,'(access(Param@2,Param@0),'(lmove(Param@2,Param@0,1,+(Param@1,neg(1))),+(Param@1,neg(+(1,1))))));;
F:
	collect(0,+(1,+(1,Param@1)))
	collect(0,Param@0)
	collect(0,1)
	collect(0,0)
