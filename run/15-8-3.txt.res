Step 1/5: Init Sampler
Finished
State type: List[4](List[4](Int[0,5]))
Env type: []



Step 2/5: Synthesize PreOrder for Plan
Finished 0.163672
Cmp:[=:fold((\ tmp0@(Int*Int) tmp1@Int -> tmp0.1),0,Param@0),<=:let getp = (\ xx@(Int*Int) -> xx.1) in let tr = map(getp,Param@0) in let trace = zip(tr,tail(tr)) in let valid = (\ z@(Int*Int) -> &&(<=(-(z.2,1),z.1),<=(z.1,+(z.2,1)))) in let f = (\ x@(Int*Int) y@Int -> ite(&&(==(y,1),valid(x)),1,0)) in let getw = (\ xxx@(Int*Int) -> xxx.2) in ite(==(fold(f,1,trace),1),neg(sum(map(getw,Param@0))),-30)]



Step 3/5: Rewrite F via PreOrders
Finished 5.009672
Cared Funtions
	fold((\ tmp0@(Int*Int) tmp1@Int -> tmp0.1),0,Param@0)
	let getp = (\ xx@(Int*Int) -> xx.1) in let tr = map(getp,Param@0) in let trace = zip(tr,tail(tr)) in let valid = (\ z@(Int*Int) -> &&(<=(-(z.2,1),z.1),<=(z.1,+(z.2,1)))) in let f = (\ x@(Int*Int) y@Int -> ite(&&(==(y,1),valid(x)),1,0)) in let getw = (\ xxx@(Int*Int) -> xxx.2) in ite(==(fold(f,1,trace),1),neg(sum(map(getw,Param@0))),-30)
State: List[4](List[4](Int[0,5]))	 Plan: (Int*Int)
T: if (==(size(Param@0),1)) then let last = head(Param@0) in foreach i in ..(0,+(size(last),-1)), collect(2,'(i,access(last,i))) else let cur = head(Param@0) in let rem = tail(Param@0) in foreach i in ..(0,+(size(cur),-1)), collect(1,'('(i,access(cur,i)),rem))
F:
	collect(0,'(Param@0.1,ite(||(||(<(1,-(Param@0.1,Param@1.1)),<(1,-(Param@1.1,Param@0.1))),<=(+(Param@1.2,*(Param@0.2,-1)),-30)),-30,+(Param@1.2,*(Param@0.2,-1)))))
	collect(0,'(Param@0,neg(Param@1)))



Step 4/5: Synthesize Eq Relation for States
Finished 0.820388
Eq:[fold((\ tmp0@List(Int) tmp1@Int -> +(1,tmp1)),0,Param@0)]



Step 5/5: Rewrite T via Eq Relation
Finished 6.072997
Cared Funtions
	fold((\ tmp0@List(Int) tmp1@Int -> +(1,tmp1)),0,Param@0)
	size(head(Param@0))
State: (Int*Int)	 Plan: (Int*Int)
T: if (==(Param@0,1)) then foreach i in ..(0,+(Param@1,-1)), collect(2,'(i,access(access(Param@2,neg(1)),i))) else foreach i in ..(0,+(Param@1,-1)), collect(1,'('(i,access(access(Param@2,neg(Param@0)),i)),'(+(Param@0,neg(1)),Param@1)))
F:
	collect(0,'(Param@0.1,ite(||(||(<(1,-(Param@0.1,Param@1.1)),<(1,-(Param@1.1,Param@0.1))),<=(+(Param@1.2,*(Param@0.2,-1)),-30)),-30,+(Param@1.2,*(Param@0.2,-1)))))
	collect(0,'(Param@0,neg(Param@1)))
