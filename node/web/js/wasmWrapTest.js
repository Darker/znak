import * as cwrap from "./wasmWrap.js"

const test = cwrap.wrap(null, "test", cwrap.bool, cwrap.int, cwrap.int);