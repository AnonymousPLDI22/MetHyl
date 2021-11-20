Step 1/5: Init Sampler
Finished
State type: BTree[6](Int[1,1000],Int[1,1000])
Env type: []



Step 2/5: Synthesize PreOrder for Plan
Finished 0.812508
Cmp:[<=:neg(bc(Param@0).1),<=:let getsum = (\ x@(Int*Int) y@(Int*Int) z@(Int*Int) -> ite(||(||(==(y.1,2),==(z.1,2)),&&(==(x.1,1),||(==(y.1,1),==(z.1,1)))),'(2,-1),'(x.1,+(+(*(x.1,x.2),y.2),z.2)))) in let init = (\ x@(Int*Int) -> '(x.1,*(x.1,x.2))) in let res = bfold(Param@0,getsum,init) in res.2]



Step 3/5: Rewrite F via PreOrders
Finished 17.193918
Cared Funtions
	neg(bc(Param@0).1)
	let getsum = (\ x@(Int*Int) y@(Int*Int) z@(Int*Int) -> ite(||(||(==(y.1,2),==(z.1,2)),&&(==(x.1,1),||(==(y.1,1),==(z.1,1)))),'(2,-1),'(x.1,+(+(*(x.1,x.2),y.2),z.2)))) in let init = (\ x@(Int*Int) -> '(x.1,*(x.1,x.2))) in let res = bfold(Param@0,getsum,init) in res.2
State: BTree[10](Int[1,1000],Int[1,1000])	 Plan: (Int*Int)
T: if (==(isleaf(Param@0),1)) then collect(1,bc(Param@0)) else let w = bc(Param@0) in let ltree = bl(Param@0) in let rtree = br(Param@0) in collect(2,'(w,ltree,rtree))
F:
	collect(0,'(0,0));collect(0,'(neg(1),Param@0));
	collect(0,'(0,ite(||(<=(Param@1.2,-1),<=(Param@2.2,-1)),-1,+(Param@1.2,Param@2.2))));collect(0,'(neg(1),ite(&&(&&(&&(<=(0,Param@2.2),<=(0,Param@1.2)),<=(0,Param@1.1)),<=(0,Param@2.1)),+(+(Param@0,Param@1.2),Param@2.2),-1)));



Step 4/5: Synthesize Eq Relation for States
Finished 3.094744
Eq:[bmatch(Param@0,Param@1)]



Step 5/5: Rewrite T via Eq Relation
Finished 10.332096
Cared Funtions
	bmatch(Param@0,Param@1)
	isleaf(Param@0)
State: (Int*Int)	 Plan: (Int*Int)
T: if (==(Param@1,1)) then collect(1,baccess(Param@0,Param@2)) else collect(2,'(baccess(Param@0,Param@2),'(bmovel(Param@0,Param@2),isleaf(subtree(1,subtree(Param@0,Param@2)))),'(bmover(Param@0,Param@2),isleaf(subtree(bmover(Param@0,Param@2),Param@2)))))
F:
	collect(0,'(0,0));collect(0,'(neg(1),Param@0));
	collect(0,'(0,ite(||(<=(Param@1.2,-1),<=(Param@2.2,-1)),-1,+(Param@1.2,Param@2.2))));collect(0,'(neg(1),ite(&&(&&(&&(<=(0,Param@2.2),<=(0,Param@1.2)),<=(0,Param@1.1)),<=(0,Param@2.1)),+(+(Param@0,Param@1.2),Param@2.2),-1)));
