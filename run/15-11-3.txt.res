Step 1/5: Init Sampler
Finished
State type: List[5](Int[0,3])
Env type: [List[16](Int[0,3]),Int[1,3],Int[0,3],Int[0,1]]



Step 2/5: Synthesize PreOrder for Plan
Finished 9.887881
Cmp:[=:fold((\ tmp0@(Int*Int) tmp1@Int -> +(tmp1,tmp0.1)),0,Param@0),<=:let g = (\ x@(Int*Int) y@(Int*Int) -> '(-(+(y.1,x.1),x.2),ite(||(<(+(y.1,x.1),x.2),==(y.2,61)),61,+(+(y.2,access(Param@1,-(+(y.1,x.1),x.2))),ite(>(x.1,Param@3),*(Param@4,-(x.1,Param@3)),0))))) in let res = fold(g,'(0,0),Param@0) in neg(res.2);]



Step 3/5: Rewrite F via PreOrders
Failed 120.015355



Step 4/5: Synthesize Eq Relation for States
Finished 0.645792
Eq:[fold((\ tmp0@Int tmp1@Int -> +(1,tmp1)),0,Param@0)]



Step 5/5: Rewrite T via Eq Relation
Finished 6.570250
Cared Funtions
	fold((\ tmp0@Int tmp1@Int -> +(1,tmp1)),0,Param@0)
	head(reverse(Param@0))
State: (Int*Int)	 Plan: List((Int*Int))
T: if (==(Param@0,0)) then collect(2,empty) else foreach i in ..(0,Param@3), collect(1,'('(i,Param@1),'(+(Param@0,neg(1)),access(Param@6,+(Param@0,neg(+(1,1)))))))
F:
	collect(0,cons(Param@0,Param@1))
	collect(0,nil('(0,0)))
