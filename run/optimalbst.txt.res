Step 1/5: Init Sampler
Finished
State type: List[7](Int[0,500])
Env type: []



Step 2/5: Synthesize PreOrder for Plan
Finished 0.154150
Cmp:[<=:let f = (\ vi@Int l@(Int*Int) r@(Int*Int) -> '(+(+(vi,l.1),r.1),+(+(+(+(vi,l.1),l.2),r.1),r.2))) in let g = (\ vl@Void -> '(0,0)) in let res = bfold(Param@0,f,g) in neg(res.2)]



Step 3/5: Rewrite F via PreOrders
Finished 22.291960
Cared Funtions
	let f = (\ vi@Int l@(Int*Int) r@(Int*Int) -> '(+(+(vi,l.1),r.1),+(+(+(+(vi,l.1),l.2),r.1),r.2))) in let g = (\ vl@Void -> '(0,0)) in let res = bfold(Param@0,f,g) in neg(res.2)
	bfold(Param@0,(\ tmp0@Int tmp1@Int tmp2@Int -> +(tmp0,+(tmp1,tmp2))),(\ tmp0@Void -> 0))
State: List[7](Int[0,500])	 Plan: (Int*Int)
T: if (==(size(Param@0),0)) then collect(2,empty) else let s = size(Param@0) in foreach i in ..(0,+(-1,s)), collect(1,'(access(Param@0,i),take(Param@0,i),drop(Param@0,+(i,1))))
F:
	collect(0,'(+(Param@1.1,+(Param@2.1,neg(+(Param@0,+(Param@1.2,Param@2.2))))),+(Param@0,+(Param@1.2,Param@2.2))))
	collect(0,'(0,0))



Step 4/5: Synthesize Eq Relation for States
Finished 1.228415
Eq:[lmatch(Param@0,Param@1),fold((\ tmp0@Int tmp1@Int -> +(1,tmp1)),0,Param@0)]



Step 5/5: Rewrite T via Eq Relation
Finished 0.852382
Cared Funtions
	lmatch(Param@0,Param@1)
	fold((\ tmp0@Int tmp1@Int -> +(1,tmp1)),0,Param@0)
State: (Int*Int)	 Plan: (Int*Int)
T: if (==(Param@1,0)) then collect(2,empty) else foreach i in ..(0,+(-1,Param@1)), collect(1,'(access(Param@2,+(i,Param@0)),'(lmove(Param@2,Param@0,0,i),i),'(lmove(Param@2,Param@0,+(1,i),Param@1),+(Param@1,neg(+(1,i))))))
F:
	collect(0,'(+(Param@1.1,+(Param@2.1,neg(+(Param@0,+(Param@1.2,Param@2.2))))),+(Param@0,+(Param@1.2,Param@2.2))))
	collect(0,'(0,0))
