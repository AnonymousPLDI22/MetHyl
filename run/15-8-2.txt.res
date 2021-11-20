Step 1/5: Init Sampler
Finished
State type: List[1000](List[6](Int[0,1000]))
Env type: []



Step 2/5: Synthesize PreOrder for Plan
Finished 0.168083
Cmp:[=:fold((\ tmp0@(Int*Int) tmp1@Int -> tmp0.1),0,Param@0),<=:let getw = (\ x@(Int*Int) -> x.2) in neg(sum(map(getw,Param@0)))]



Step 3/5: Rewrite F via PreOrders
Finished 15.735425
Cared Funtions
	fold((\ tmp0@(Int*Int) tmp1@Int -> tmp0.1),0,Param@0)
	let getw = (\ x@(Int*Int) -> x.2) in neg(sum(map(getw,Param@0)))
	head(Param@0).2
State: List[1000](List[1000](Int[0,1000]))	 Plan: (Int*Int*Int)
T: if (==(size(Param@0),1)) then let last = head(Param@0) in foreach i in ..(0,+(size(last),-1)), collect(2,'(i,access(last,i))) else let cur = head(Param@0) in let rem = tail(Param@0) in foreach i in ..(0,+(size(cur),-1)), collect(1,'('(i,access(cur,i)),rem))
F:
	if (&&(<=(+(Param@1.1,-1),Param@0.1),<=(Param@0.1,+(Param@1.1,1)))) then collect(0,'(Param@0.1,+(Param@1.2,neg(Param@0.2)),Param@0.2))
	collect(0,'(Param@0,neg(Param@1),Param@1))



Step 4/5: Synthesize Eq Relation for States
Finished 3.359224
Eq:[fold((\ tmp0@List(Int) tmp1@Int -> +(1,tmp1)),0,Param@0)]



Step 5/5: Rewrite T via Eq Relation
Finished 6.273391
Cared Funtions
	fold((\ tmp0@List(Int) tmp1@Int -> +(1,tmp1)),0,Param@0)
	size(head(Param@0))
State: (Int*Int)	 Plan: (Int*Int*Int)
T: if (==(Param@0,1)) then foreach i in ..(0,+(Param@1,-1)), collect(2,'(i,access(access(Param@2,neg(1)),i))) else foreach i in ..(0,+(Param@1,-1)), collect(1,'('(i,access(access(Param@2,neg(Param@0)),i)),'(+(Param@0,neg(1)),Param@1)))
F:
	if (&&(<=(+(Param@1.1,-1),Param@0.1),<=(Param@0.1,+(Param@1.1,1)))) then collect(0,'(Param@0.1,+(Param@1.2,neg(Param@0.2)),Param@0.2))
	collect(0,'(Param@0,neg(Param@1),Param@1))
