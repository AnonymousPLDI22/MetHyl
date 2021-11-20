Step 1/5: Init Sampler
Finished
State type: List[13](Int[0,13])
Env type: [Int[0,1000]]



Step 2/5: Synthesize PreOrder for Plan
Finished 0.052880
Cmp:[<=:-(sum(Param@0),*(size(Param@0),Param@1))]



Step 3/5: Rewrite F via PreOrders
Finished 16.989816
Cared Funtions
	-(sum(Param@0),*(size(Param@0),Param@1))
State: List[1000](Int[0,1000])	 Plan: Int
T: if (==(size(Param@0),0)) then collect(1,empty) else foreach i in ..(1,size(Param@0)), collect(2,'(access(Param@0,+(i,-1)),take(Param@0,-(size(Param@0),i))))
F:
	collect(0,0)
	collect(0,+(Param@0,-(Param@1,Param@2)))



Step 4/5: Synthesize Eq Relation for States
Finished 0.924204
Eq:[fold((\ tmp0@Int tmp1@Int -> +(1,tmp1)),0,Param@0)]



Step 5/5: Rewrite T via Eq Relation
Finished 0.471622
Cared Funtions
	fold((\ tmp0@Int tmp1@Int -> +(1,tmp1)),0,Param@0)
State: Int	 Plan: Int
T: if (==(Param@0,0)) then collect(1,empty) else foreach i in ..(1,Param@0), collect(2,'(access(Param@2,+(-1,i)),-(Param@0,i)))
F:
	collect(0,0)
	collect(0,+(Param@0,-(Param@1,Param@2)))
