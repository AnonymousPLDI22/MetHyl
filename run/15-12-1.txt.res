Step 1/5: Init Sampler
Finished
State type: (List[5](List[100]((Int[0,100]*Int[0,100])))*List[0]((Int[0,100]*Int[0,100])))
Env type: [Int[0,100]]



Step 2/5: Synthesize PreOrder for Plan
Finished 0.031664
Cmp:[<=:let getv = (\ x@(Int*Int) -> x.2) in sum(map(getv,Param@0))]



Step 3/5: Rewrite F via PreOrders
Finished 30.354526
Cared Funtions
	let getv = (\ x@(Int*Int) -> x.2) in sum(map(getv,Param@0))
State: (List[5](List[100]((Int[0,100]*Int[0,100])))*List[0]((Int[0,100]*Int[0,100])))	 Plan: Int
T: if (==(size(Param@0),0)) then collect(2,empty) else let h = head(Param@0) in let rem = tail(Param@0) in let getw = (\ x@(Int*Int) -> x.1) in let totw = sum(map(getw,Param@1)) in foreach i in ..(0,+(size(h),-1)), let cur = access(h,i) in if (<=(+(totw,cur.1),Param@2)) then collect(1,'(cur,'(rem,cons(cur,Param@1))))
F:
	collect(0,+(Param@0.2,Param@1))
	collect(0,0)



Step 4/5: Synthesize Eq Relation for States
Finished 9.337365
Eq:[fold((\ tmp0@List((Int*Int)) tmp1@Int -> +(1,tmp1)),0,Param@0),fold((\ tmp0@(Int*Int) tmp1@Int -> +(tmp1,tmp0.1)),0,Param@1)]



Step 5/5: Rewrite T via Eq Relation
Finished 15.142120
Cared Funtions
	fold((\ tmp0@List((Int*Int)) tmp1@Int -> +(1,tmp1)),0,Param@0)
	fold((\ tmp0@(Int*Int) tmp1@Int -> +(tmp1,tmp0.1)),0,Param@1)
	size(head(Param@0))
State: (Int*Int*Int)	 Plan: Int
T: if (==(Param@0,0)) then collect(2,empty) else foreach i in ..(0,+(Param@2,-1)), if (<=(+(Param@1,'(access(access(Param@4.1,neg(Param@0)),i).1,access(access(Param@4.1,neg(Param@0)),i).2).1),Param@3)) then collect(1,'('(access(access(Param@4.1,neg(Param@0)),i).1,access(access(Param@4.1,neg(Param@0)),i).2),'(+(Param@0,neg(1)),+(Param@1,access(access(Param@4.1,neg(Param@0)),i).1),Param@2)))
F:
	collect(0,+(Param@0.2,Param@1))
	collect(0,0)
