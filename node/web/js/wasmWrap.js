/**
 * @template T
 */
class CType {
    /**
     * 
     * @param {string} name name for emscripten to use
     * @param {T} typehelper pass this to tell JSDoc what JS type to infer
     */
    constructor(name, typehelper) {
        this.name = name;
    }
}

const bool = new CType("bool", true);
const int = new CType("int", 1);
const short = new CType("short", 1);

export {bool, int, short};

/**
 * @template TRet
 * @template T1
 * @template T2
 * @template T3
 * @template T4
 * @template T5
 * @template T6
 * 
 * @param {any} Module wasm main module container
 * @param {string} functionName 
 * @param {CType<TRet>} returnVal return val of your C routine
 * @param {CType<T1>} arg1 
 * @param {CType<T2>} arg2 
 * @param {CType<T3>} arg3 
 * @param {CType<T4>} arg4 
 * @param {CType<T5>} arg5 
 * @param {CType<T6>} arg6 
 * 
 * @returns {(arg1: T1, arg2: T2, arg3: T3, arg4: T4, arg5: T5) => TRet}
 */
export function wrap(Module, functionName, returnVal, arg1, arg2, arg3, arg4, arg5, arg6) {


}