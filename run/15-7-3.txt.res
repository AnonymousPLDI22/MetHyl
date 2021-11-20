Step 1/5: Init Sampler
Finished
State type: (Int[0,0]*List[6](Int[0,1]))
Env type: [List[3](List[3]((Int[0,1]*Int[0,2])))]



Step 2/5: Synthesize PreOrder for Plan
Finished 0.307451
Cmp:[<=:let pp = zip(Param@0.1,tail(Param@0.1)) in let edge = (\ x@(Int*Int) -> access(access(Param@1,x.1),x.2)) in let getc = (\ x@(Int*Int) -> x.1) in let getw = (\ x@(Int*Int) -> x.2) in let cp = map(getc,map(edge,pp)) in ite(==(cp,Param@0.2),sum(map(getw,map(edge,pp))),-40)]



Step 3/5: Rewrite F via PreOrders
Finished 7.503341
Cared Funtions
	let pp = zip(Param@0.1,tail(Param@0.1)) in let edge = (\ x@(Int*Int) -> access(access(Param@1,x.1),x.2)) in let getc = (\ x@(Int*Int) -> x.1) in let getw = (\ x@(Int*Int) -> x.2) in let cp = map(getc,map(edge,pp)) in ite(==(cp,Param@0.2),sum(map(getw,map(edge,pp))),-40)
	fold((\ tmp0@Int tmp1@Int -> tmp0),0,Param@0.1)
State: (Int[0,0]*List[6](Int[0,1]))	 Plan: (Int*Int)
T: if (==(size(Param@1),0)) then collect(1,Param@0) else foreach i in ..(0,+(size(Param@2),-1)), let e = access(access(Param@2,Param@0),i) in collect(2,'(Param@0,head(Param@1),'(i,tail(Param@1))));
F:
	collect(0,'(0,Param@0))
	collect(0,'(ite(&&(&&(<=(0,+(Param@2.1,access(access(Param@3,Param@0),Param@2.2).2)),<=(Param@1,access(access(Param@3,Param@0),Param@2.2).1)),<=(access(access(Param@3,Param@0),Param@2.2).1,Param@1)),+(Param@2.1,access(access(Param@3,Param@0),Param@2.2).2),-40),Param@0))



Step 4/5: Synthesize Eq Relation for States
Finished 0.732307
Eq:[Param@0,fold((\ tmp0@Int tmp1@Int -> +(1,tmp1)),0,Param@1)]



Step 5/5: Rewrite T via Eq Relation
Finished 0.426339
Cared Funtions
	Param@0
	fold((\ tmp0@Int tmp1@Int -> +(1,tmp1)),0,Param@1)
State: (Int[0,0]*Int)	 Plan: (Int*Int)
T: if (==(Param@1,0)) then collect(1,Param@0) else foreach i in ..(0,+(size(Param@2),-1)), collect(2,'(Param@0,access(Param@3.2,neg(Param@1)),'(i,+(Param@1,neg(1)))));
F:
	collect(0,'(0,Param@0))
	collect(0,'(ite(&&(&&(<=(0,+(Param@2.1,access(access(Param@3,Param@0),Param@2.2).2)),<=(Param@1,access(access(Param@3,Param@0),Param@2.2).1)),<=(access(access(Param@3,Param@0),Param@2.2).1,Param@1)),+(Param@2.1,access(access(Param@3,Param@0),Param@2.2).2),-40),Param@0))
