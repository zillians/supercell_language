class QWE
{
    var x:int32;

    static function f():void
    {
        @static_test { expect_message={ level="LEVEL_ERROR", id="INVALID_NONSTATIC_REF", parameters={ var_id="x" } } }
        x=13;
    }
}