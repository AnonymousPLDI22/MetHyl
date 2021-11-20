Step 1/5: Init Sampler
Finished
State type: (List[10](Int[0,5])*List[0](Int[0,5]))
Env type: [Int[5,10]]



Step 2/5: Synthesize PreOrder for Plan
Finished 0.032738
Cmp:[<=:let f = (\ x@Int -> pow(x,3)) in neg(sum(map(f,Param@0)));]



Step 3/5: Rewrite F via PreOrders
Failed 121.130665



Step 4/5: Synthesize Eq Relation for States
Finished 6.261677
Eq:[fold((\ tmp0@Int tmp1@Int -> +(1,tmp1)),0,Param@1),fold((\ tmp0@Int tmp1@Int -> +(1,tmp1)),0,Param@0)]



Step 5/5: Rewrite T via Eq Relation
Finished 15.115191
Cared Funtions
	fold((\ tmp0@Int tmp1@Int -> +(1,tmp1)),0,Param@1)
	fold((\ tmp0@Int tmp1@Int -> +(1,tmp1)),0,Param@0)
	sum(Param@1)
State: (Int*Int*Int)	 Plan: List(Int)
T: if (==(Param@1,0)) then if (<=(+(+(Param@2,Param@0),-1),Param@3)) then collect(3,empty); else if (>(Param@0,0)) then collect(1,'(+(+(+(Param@3,*(Param@0,-1)),*(Param@2,-1)),1),'(0,Param@1,0)));if (<=(+(+(Param@2,Param@0),access(Param@4.1,neg(Param@1))),Param@3)) then collect(2,'(+(Param@0,1),+(Param@1,-1),+(Param@2,access(Param@4.1,neg(Param@1)))));
F:
	collect(0,cons(Param@0,Param@1))
	collect(0,Param@0)
	collect(0,nil(0))
