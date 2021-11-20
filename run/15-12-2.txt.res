Step 1/5: Init Sampler
Finished
State type: List[5](List[100]((Int[0,100]*Int[0,100])))
Env type: [Int[0,100]]



Step 2/5: Synthesize PreOrder for Plan
Finished 2.101741
Cmp:[<=:neg(fold((\ tmp0@(Int*Int) tmp1@Int -> +(tmp1,tmp0.1)),0,Param@0)),<=:let getv = (\ x@(Int*Int) -> x.2) in sum(map(getv,Param@0))]



Step 3/5: Rewrite F via PreOrders
Finished 17.858060
Cared Funtions
	neg(fold((\ tmp0@(Int*Int) tmp1@Int -> +(tmp1,tmp0.1)),0,Param@0))
	let getv = (\ x@(Int*Int) -> x.2) in sum(map(getv,Param@0))
State: List[5](List[100]((Int[0,100]*Int[0,100])))	 Plan: (Int*Int)
T: if (==(size(Param@0),0)) then collect(2,empty) else let h = head(Param@0) in let rem = tail(Param@0) in foreach i in ..(0,+(size(h),-1)), collect(1,'(access(h,i),rem))
F:
	if (<=(+(Param@0.1,*(Param@1.1,-1)),Param@2)) then collect(0,'(+(Param@1.1,neg(Param@0.1)),+(Param@0.2,Param@1.2)))
	collect(0,'(0,0))



Step 4/5: Synthesize Eq Relation for States
Finished 0.355493
Eq:[fold((\ tmp0@List((Int*Int)) tmp1@Int -> +(1,tmp1)),0,Param@0)]



Step 5/5: Rewrite T via Eq Relation
Finished 7.273329
Cared Funtions
	fold((\ tmp0@List((Int*Int)) tmp1@Int -> +(1,tmp1)),0,Param@0)
	size(head(Param@0))
State: (Int*Int)	 Plan: (Int*Int)
T: if (==(Param@0,0)) then collect(2,empty) else foreach i in ..(0,+(Param@1,-1)), collect(1,'('(access(access(Param@3,neg(Param@0)),i).1,access(access(Param@3,neg(Param@0)),i).2),'(+(Param@0,neg(1)),Param@1)))
F:
	if (<=(+(Param@0.1,*(Param@1.1,-1)),Param@2)) then collect(0,'(+(Param@1.1,neg(Param@0.1)),+(Param@0.2,Param@1.2)))
	collect(0,'(0,0))
