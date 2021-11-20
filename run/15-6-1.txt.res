Step 1/5: Init Sampler
Finished
State type: (BTree[8](Int[1,1000],Int[1,1000])*List[0](Int[0,1]))
Env type: []



Step 2/5: Synthesize PreOrder for Plan
Finished 0.549205
Cmp:[<=:let getsum = (\ x@(Int*Int) y@Int z@Int -> +(+(*(x.1,x.2),y),z)) in let init = (\ x@(Int*Int) -> *(x.1,x.2)) in bfold(Param@0,getsum,init)]



Step 3/5: Rewrite F via PreOrders
Finished 2.728657
Cared Funtions
	let getsum = (\ x@(Int*Int) y@Int z@Int -> +(+(*(x.1,x.2),y),z)) in let init = (\ x@(Int*Int) -> *(x.1,x.2)) in bfold(Param@0,getsum,init)
State: (BTree[10](Int[1,1000],Int[1,1000])*List[0](Int[0,1]))	 Plan: Int
T: if (==(isleaf(Param@0),1)) then collect(1,'(0,bc(Param@0)));if (==(head(Param@1),0)) then collect(1,'(1,bc(Param@0))); else let w = bc(Param@0) in let ltree = bl(Param@0) in let rtree = br(Param@0) in collect(2,'('(0,w),'(ltree,cons(0,Param@1)),'(rtree,cons(0,Param@1))));if (==(size(Param@1),0)) then collect(2,'('(1,w),'(ltree,cons(1,Param@1)),'(rtree,cons(1,Param@1)))); else if (==(head(Param@1),0)) then collect(2,'('(1,w),'(ltree,cons(1,Param@1)),'(rtree,cons(1,Param@1))));;
F:
	collect(0,*(Param@0,Param@1))
	collect(0,+(Param@1,+(Param@2,*(Param@0.1,Param@0.2))))



Step 4/5: Synthesize Eq Relation for States
Finished 1.832714
Eq:[fold((\ tmp0@Int tmp1@Int -> tmp0),0,Param@1),bmatch(Param@0,Param@2.1)]



Step 5/5: Rewrite T via Eq Relation
Finished 36.087769
Cared Funtions
	fold((\ tmp0@Int tmp1@Int -> tmp0),0,Param@1)
	bmatch(Param@0,Param@2.1)
	isleaf(Param@0)
	size(Param@1)
State: (Int*Int*Int*Int)	 Plan: Int
T: if (==(Param@2,1)) then collect(1,'(0,baccess(Param@1,Param@4.1)));if (==(Param@0,0)) then collect(1,'(1,baccess(Param@1,Param@4.1))); else collect(2,'('(0,baccess(Param@1,Param@4.1)),'(0,bmovel(Param@1,Param@4.1),isleaf(subtree(1,subtree(Param@1,Param@4.1))),+(1,Param@3)),'(0,bmover(Param@1,Param@4.1),isleaf(subtree(bmover(Param@1,Param@4.1),Param@4.1)),+(1,Param@3))));if (==(Param@3,0)) then collect(2,'('(1,baccess(0,Param@4.1)),'(1,1,isleaf(subtree(1,Param@4.1)),1),'(1,bmover(0,Param@4.1),isleaf(subtree(bmover(0,Param@4.1),Param@4.1)),1))); else if (==(Param@0,0)) then collect(2,'('(1,baccess(Param@1,Param@4.1)),'(1,bmovel(Param@1,Param@4.1),isleaf(subtree(1,subtree(Param@1,Param@4.1))),+(1,Param@3)),'(1,bmover(Param@1,Param@4.1),isleaf(subtree(bmover(Param@1,Param@4.1),Param@4.1)),+(1,Param@3))));;
F:
	collect(0,*(Param@0,Param@1))
	collect(0,+(Param@1,+(Param@2,*(Param@0.1,Param@0.2))))
