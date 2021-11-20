Step 1/5: Init Sampler
Finished
State type: List[6](Int[0,1000])
Env type: []



Step 2/5: Synthesize PreOrder for Plan
Finished 0.053513
Cmp:[<=:let getsum = (\ x@Void y@(Int*Int) z@(Int*Int) -> '(+(+(+(y.1,y.2),z.1),z.2),+(y.2,z.2))) in let init = (\ x@Int -> '(0,x)) in let res = bfold(Param@0,getsum,init) in neg(res.1)]



Step 3/5: Rewrite F via PreOrders
Finished 1.480284
Cared Funtions
	let getsum = (\ x@Void y@(Int*Int) z@(Int*Int) -> '(+(+(+(y.1,y.2),z.1),z.2),+(y.2,z.2))) in let init = (\ x@Int -> '(0,x)) in let res = bfold(Param@0,getsum,init) in neg(res.1)
	bfold(Param@0,(\ tmp0@Void tmp1@Int tmp2@Int -> +(tmp1,tmp2)),(\ tmp0@Int -> tmp0))
State: List[6](Int[0,1000])	 Plan: (Int*Int)
T: let m = size(Param@0) in if (==(m,1)) then collect(2,head(Param@0)) else foreach p in ..(1,+(m,-1)), let y = take(Param@0,p) in let z = drop(Param@0,p) in collect(1,'(y,z));
F:
	collect(0,'(+(Param@0.1,+(Param@1.1,neg(+(Param@0.2,Param@1.2)))),+(Param@0.2,Param@1.2)))
	collect(0,'(0,Param@0))



Step 4/5: Synthesize Eq Relation for States
Finished 1.534778
Eq:[lmatch(Param@0,Param@1),fold((\ tmp0@Int tmp1@Int -> +(1,tmp1)),0,Param@0)]



Step 5/5: Rewrite T via Eq Relation
Finished 1.733672
Cared Funtions
	lmatch(Param@0,Param@1)
	fold((\ tmp0@Int tmp1@Int -> +(1,tmp1)),0,Param@0)
State: (Int*Int)	 Plan: (Int*Int)
T: if (==(Param@1,1)) then collect(2,access(Param@2,Param@0)) else foreach p in ..(1,+(Param@1,-1)), collect(1,'('(lmove(Param@2,Param@0,0,p),p),'(lmove(Param@2,Param@0,p,Param@1),+(Param@1,neg(p)))));
F:
	collect(0,'(+(Param@0.1,+(Param@1.1,neg(+(Param@0.2,Param@1.2)))),+(Param@0.2,Param@1.2)))
	collect(0,'(0,Param@0))
