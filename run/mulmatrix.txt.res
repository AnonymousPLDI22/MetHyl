Step 1/5: Init Sampler
Finished
State type: List[11](Int[0,11])
Env type: []



Step 2/5: Synthesize PreOrder for Plan
Finished 0.117124
Cmp:[<=:let getsum = (\ x@Void y@(Int*Int*Int) z@(Int*Int*Int) -> '(y.1,z.2,+(+(y.3,z.3),*(*(y.1,z.1),z.2)))) in let init = (\ x@(Int*Int) -> '(x.1,x.2,0)) in let res = bfold(Param@0,getsum,init) in neg(res.3)]



Step 3/5: Rewrite F via PreOrders
Finished 10.402331
Cared Funtions
	let getsum = (\ x@Void y@(Int*Int*Int) z@(Int*Int*Int) -> '(y.1,z.2,+(+(y.3,z.3),*(*(y.1,z.1),z.2)))) in let init = (\ x@(Int*Int) -> '(x.1,x.2,0)) in let res = bfold(Param@0,getsum,init) in neg(res.3)
	bfold(Param@0,(\ tmp0@Void tmp1@(Int*Int) tmp2@(Int*Int) -> tmp1),(\ tmp0@(Int*Int) -> tmp0)).1
	bfold(Param@0,(\ tmp0@Void tmp1@(Int*Int) tmp2@(Int*Int) -> tmp2),(\ tmp0@(Int*Int) -> tmp0)).2
State: List[1000](Int[1,1000])	 Plan: (Int*Int*Int)
T: if (==(size(Param@0),2)) then collect(1,'(head(Param@0),access(Param@0,1))) else foreach i in ..(2,+(size(Param@0),-1)), collect(2,'(take(Param@0,i),drop(Param@0,-(i,1))))
F:
	collect(0,'(0,Param@0,Param@1));
	collect(0,'(+(+(Param@0.1,Param@1.1),*(*(Param@0.2,*(Param@1.2,Param@1.3)),-1)),Param@0.2,Param@1.3));



Step 4/5: Synthesize Eq Relation for States
Finished 2.831958
Eq:[lmatch(Param@0,Param@1),fold((\ tmp0@Int tmp1@Int -> +(1,tmp1)),0,Param@0)]



Step 5/5: Rewrite T via Eq Relation
Finished 2.093796
Cared Funtions
	lmatch(Param@0,Param@1)
	fold((\ tmp0@Int tmp1@Int -> +(1,tmp1)),0,Param@0)
State: (Int*Int)	 Plan: (Int*Int*Int)
T: if (==(Param@1,2)) then collect(1,'(access(Param@2,Param@0),access(Param@2,+(1,Param@0)))) else foreach i in ..(2,+(Param@1,-1)), collect(2,'('(lmove(Param@2,Param@0,0,i),i),'(lmove(Param@2,Param@0,-(i,1),Param@1),+(1,-(Param@1,i)))))
F:
	collect(0,'(0,Param@0,Param@1));
	collect(0,'(+(+(Param@0.1,Param@1.1),*(*(Param@0.2,*(Param@1.2,Param@1.3)),-1)),Param@0.2,Param@1.3));
