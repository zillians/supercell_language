function f():void
{
    const x:int32 = 13;

    @static_test { expect_message={ level="LEVEL_ERROR", id="WRITE_CONST", parameters={ var_id="x" } } }
    x=17;
}
