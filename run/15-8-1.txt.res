Step 1/5: Init Sampler
Finished
State type: (List[0](Int[0,1000])*List[1000](List[5](Int[0,1000])))
Env type: []



Step 2/5: Synthesize PreOrder for Plan
Finished 0.035851
Cmp:[<=:let getw = (\ x@(Int*Int) -> x.2) in neg(sum(map(getw,Param@0)))]



Step 3/5: Rewrite F via PreOrders
Finished 10.404926
Cared Funtions
	let getw = (\ x@(Int*Int) -> x.2) in neg(sum(map(getw,Param@0)))
State: (List[0](Int[0,1000])*List[1000](List[1000](Int[0,1000])))	 Plan: Int
T: if (==(size(Param@1),0)) then collect(2,empty) else let h = head(Param@1) in let rem = tail(Param@1) in if (==(size(Param@0),0)) then foreach i in ..(0,+(size(h),-1)), collect(1,'('(i,access(h,i)),'(cons(i,nil(0)),rem))) else let la = head(Param@0) in foreach i in ..(max(0,-(la,1)),min(+(la,1),+(size(h),-1))), collect(1,'('(i,access(h,i)),'(cons(i,Param@0),rem)))
F:
	collect(0,+(Param@1,neg(Param@0.2)))
	collect(0,0)



Step 4/5: Synthesize Eq Relation for States
Finished 1.951996
Eq:[lmatch(Param@1,Param@2.2),fold((\ tmp0@Int tmp1@Int -> tmp0),neg(1),Param@0)]



Step 5/5: Rewrite T via Eq Relation
Finished 18.910963
Cared Funtions
	lmatch(Param@1,Param@2.2)
	fold((\ tmp0@Int tmp1@Int -> tmp0),neg(1),Param@0)
	size(Param@1)
State: (Int*Int*Int)	 Plan: Int
T: if (==(Param@2,0)) then collect(2,empty) else if (==(ite(&&(&&(<=(Param@2,1),<=(Param@0,0)),<=(0,Param@1)),Param@2,Param@0),0)) then foreach i in ..(0,+(Param@2,-1)), collect(1,'('(i,access(access(Param@3.2,0),i)),'(lmove(Param@3.2,0,1,Param@2),i,+(Param@2,Param@1)))) else foreach i in ..(max(0,-(Param@1,1)),min(+(Param@1,1),+(+(Param@0,Param@2),-1))), collect(1,'('(i,access(access(Param@3.2,Param@0),i)),'(lmove(Param@3.2,Param@0,1,Param@2),i,+(Param@2,neg(1)))))
F:
	collect(0,+(Param@1,neg(Param@0.2)))
	collect(0,0)
