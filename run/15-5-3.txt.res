Step 1/5: Init Sampler
Finished
State type: (List[6](Int[1,2])*List[0](Int[1,2]))
Env type: [List[6](Int[0,5]),List[3](Int[1,2])]



Step 2/5: Synthesize PreOrder for Plan
Failed 121.348426



Step 3/5: Rewrite F via PreOrders
Failed 0.000079



Step 4/5: Synthesize Eq Relation for States
Finished 3.566699
Eq:[fold((\ tmp0@Int tmp1@Int -> +(1,tmp1)),0,Param@0),fold((\ tmp0@Int tmp1@Int -> +(1,tmp1)),0,Param@1)]



Step 5/5: Rewrite T via Eq Relation
Finished 31.302963
Cared Funtions
	fold((\ tmp0@Int tmp1@Int -> +(1,tmp1)),0,Param@0)
	fold((\ tmp0@Int tmp1@Int -> +(1,tmp1)),0,Param@1)
	head(tail(Param@0))
State: (Int*Int*Int)	 Plan: (List(Int)*List(Int))
T: if (&&(==(Param@0,0),==(Param@1,size(Param@4)))) then collect(4,empty);if (&&(>(Param@0,0),<(Param@1,size(Param@4)))) then collect(2,'(0,access(Param@5.1,neg(Param@0)),'(+(Param@0,neg(1)),+(1,Param@1),access(Param@5.1,+(1,+(1,neg(Param@0)))))));collect(2,'(1,access(Param@4,Param@1),'(+(Param@0,neg(1)),+(1,Param@1),access(Param@5.1,+(1,+(1,neg(Param@0)))))));;if (>(Param@0,0)) then collect(1,'(2,'(+(Param@0,neg(1)),Param@1,access(Param@5.1,+(1,+(1,neg(Param@0)))))));;if (<(Param@1,size(Param@4))) then collect(2,'(3,access(Param@4,Param@1),'(Param@0,+(1,Param@1),Param@2)));;if (&&(>(Param@0,1),<(+(Param@1,1),size(Param@4)))) then collect(3,'(4,Param@2,access(Param@5.1,neg(Param@0)),'(+(Param@0,neg(2)),+(2,Param@1),access(Param@5.1,neg(1)))));;if (&&(>(Param@0,0),==(Param@1,size(Param@4)))) then collect(1,'(5,'(0,Param@1,0)));
F:
	collect(0,'(cons(Param@0,Param@1.1),Param@1.2))
	collect(0,'(cons(Param@0,Param@2.1),cons(Param@1,Param@2.2)))
	collect(0,'(cons(Param@0,Param@3.1),cons(Param@1,cons(Param@2,Param@3.2))))
	collect(0,'(nil(0),nil(0)))
