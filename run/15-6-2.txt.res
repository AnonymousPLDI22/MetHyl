Step 1/5: Init Sampler
Finished
State type: BTree[7](Int[1,1000],Int[1,1000])
Env type: []



Step 2/5: Synthesize PreOrder for Plan
Finished 0.406394
Cmp:[<=:neg(bc(Param@0).1),<=:let getsum = (\ x@(Int*Int) y@Int z@Int -> +(+(*(x.1,x.2),y),z)) in let init = (\ x@(Int*Int) -> *(x.1,x.2)) in bfold(Param@0,getsum,init)]



Step 3/5: Rewrite F via PreOrders
Finished 18.079823
Cared Funtions
	neg(bc(Param@0).1)
	let getsum = (\ x@(Int*Int) y@Int z@Int -> +(+(*(x.1,x.2),y),z)) in let init = (\ x@(Int*Int) -> *(x.1,x.2)) in bfold(Param@0,getsum,init)
	bc(Param@0).2
State: BTree[10](Int[1,1000],Int[1,1000])	 Plan: (Int*Int*Int)
T: if (==(isleaf(Param@0),1)) then collect(1,bc(Param@0)) else let w = bc(Param@0) in let ltree = bl(Param@0) in let rtree = br(Param@0) in collect(2,'(w,ltree,rtree))
F:
	collect(0,'(0,0,Param@0));collect(0,'(neg(1),Param@0,Param@0));
	if (&&(==(*(Param@1.1,-1),0),==(*(Param@2.1,-1),0))) then collect(0,'(neg(1),+(Param@0,+(Param@1.2,Param@2.2)),Param@0));collect(0,'(0,+(Param@1.2,Param@2.2),Param@0));



Step 4/5: Synthesize Eq Relation for States
Finished 1.110761
Eq:[bmatch(Param@0,Param@1)]



Step 5/5: Rewrite T via Eq Relation
Finished 8.545350
Cared Funtions
	bmatch(Param@0,Param@1)
	isleaf(Param@0)
State: (Int*Int)	 Plan: (Int*Int*Int)
T: if (==(Param@1,1)) then collect(1,baccess(Param@0,Param@2)) else collect(2,'(baccess(Param@0,Param@2),'(bmovel(Param@0,Param@2),isleaf(subtree(1,subtree(Param@0,Param@2)))),'(bmover(Param@0,Param@2),isleaf(subtree(bmover(Param@0,Param@2),Param@2)))))
F:
	collect(0,'(0,0,Param@0));collect(0,'(neg(1),Param@0,Param@0));
	if (&&(==(*(Param@1.1,-1),0),==(*(Param@2.1,-1),0))) then collect(0,'(neg(1),+(Param@0,+(Param@1.2,Param@2.2)),Param@0));collect(0,'(0,+(Param@1.2,Param@2.2),Param@0));
