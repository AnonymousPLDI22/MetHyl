Step 1/5: Init Sampler
Finished
State type: (List[5](Int[0,3])*List[0]((Int[0,3]*Int[0,3])))
Env type: [List[16](Int[0,3]),Int[1,3],Int[0,3],Int[0,1]]



Step 2/5: Synthesize PreOrder for Plan
Finished 0.130563
Cmp:[<=:let g = (\ x@(Int*Int) y@Int -> +(+(y,access(Param@1,x.2)),ite(>(x.1,Param@3),*(Param@4,-(x.1,Param@3)),0))) in let res = fold(g,0,Param@0) in neg(res);]



Step 3/5: Rewrite F via PreOrders
Finished 14.714030
Cared Funtions
	let g = (\ x@(Int*Int) y@Int -> +(+(y,access(Param@1,x.2)),ite(>(x.1,Param@3),*(Param@4,-(x.1,Param@3)),0))) in let res = fold(g,0,Param@0) in neg(res);
State: (List[5](Int[0,3])*List[0]((Int[0,3]*Int[0,3])))	 Plan: Int
T: if (==(size(Param@0),0)) then collect(2,empty) else let la = head(Param@0) in let rem = tail(Param@0) in let getrem = (\ x@(Int*Int) -> -(x.1,x.2)) in let num = sum(map(getrem,Param@1)) in foreach i in ..(0,Param@3), if (<=(la,+(i,num))) then collect(1,'('(i,-(+(i,num),la)),'(rem,cons('(i,la),Param@1))))
F:
	collect(0,ite(&&(<=(1,Param@5),<=(+(+(+(Param@4,*(Param@0.1,-1)),Param@1),*(access(Param@2,Param@0.2),-1)),+(Param@1,*(access(Param@2,Param@0.2),-1)))),+(+(+(Param@4,*(Param@0.1,-1)),Param@1),*(access(Param@2,Param@0.2),-1)),+(Param@1,*(access(Param@2,Param@0.2),-1))))
	collect(0,0)



Step 4/5: Synthesize Eq Relation for States
Finished 112.085846
Eq:[fold((\ tmp0@Int tmp1@Int -> +(1,tmp1)),0,Param@0),fold((\ tmp0@(Int*Int) tmp1@Int -> +(tmp1,tmp0.1)),0,Param@1)]



Step 5/5: Rewrite T via Eq Relation
Failed 15.021596
