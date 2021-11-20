Step 1/5: Init Sampler
Finished
State type: List[4](List[4](Int[1,3]))
Env type: [Int[0,2],Int[0,2]]



Step 2/5: Synthesize PreOrder for Plan
Finished 0.126513
Cmp:[=:fold((\ tmp0@(Int*Int) tmp1@Int -> tmp0.1),0,Param@0),<=:let f = (\ x@(Int*Int) y@(Int*Int) -> '(x.1,ite(==(y.1,x.1),*(-(y.2,Param@1),x.2),*(-(y.2,Param@2),x.2)))) in let res = fold(f,'(-1,3),Param@0) in res.2]



Step 3/5: Rewrite F via PreOrders
Finished 21.737148
Cared Funtions
	fold((\ tmp0@(Int*Int) tmp1@Int -> tmp0.1),0,Param@0)
	let f = (\ x@(Int*Int) y@(Int*Int) -> '(x.1,ite(==(y.1,x.1),*(-(y.2,Param@1),x.2),*(-(y.2,Param@2),x.2)))) in let res = fold(f,'(-1,3),Param@0) in res.2
	fold((\ tmp0@(Int*Int) tmp1@Int -> 0),1,Param@0)
State: List[4](List[4](Int[1,3]))	 Plan: (Int*Int*Int)
T: if (==(size(Param@0),0)) then collect(2,empty) else let la = head(reverse(Param@0)) in let rem = reverse(tail(reverse(Param@0))) in foreach j in ..(0,+(size(head(Param@0)),-1)), collect(1,'('(j,access(la,j)),rem))
F:
	collect(0,'(Param@0.1,ite(&&(==(Param@0.1,Param@1.1),<=(Param@1.3,0)),+(*(*(Param@2,Param@0.2),-1),*(Param@0.2,Param@1.2)),+(*(*(Param@3,Param@0.2),-1),*(Param@0.2,Param@1.2))),0))
	collect(0,'(0,3,1))



Step 4/5: Synthesize Eq Relation for States
Finished 0.481486
Eq:[fold((\ tmp0@List(Int) tmp1@Int -> +(1,tmp1)),0,Param@0)]



Step 5/5: Rewrite T via Eq Relation
Failed 11.746418
