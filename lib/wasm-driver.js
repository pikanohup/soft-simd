const fs = require('fs')
const util = require('util')
const { AsyncClass } = require('./async-class')

const readFile = util.promisify(fs.readFile)

class WASMDriver extends AsyncClass {
  constructor (moduleName) {
    super(async () => {
      try {
        // const wasmBinary = await readFile(`./bin/${moduleName}.wasm`)
        const Module = require(`../bin/${moduleName}`)

        const module = await Module({
          // locateFile (path) {
          //   if (path.endsWith('.wasm')) {
          //     return wasmBinary
          //   }
          //   return path
          // }
        })

        this._module = module
        this._name = moduleName

        this._wrapApi()
      } catch (error) {
        // XXX
        throw error
      }
    })
  }

  get module () {
    return this._name
  }

  _wrapApi () {
    this._api = []
    for (const key in this._module) {
      const apiPattern = /^_([a-z][A-Za-z0-9]*)/
      const match = apiPattern.exec(key)
      if (match) {
        // XXX
        this[match[1]] = (...args) => this._module[match[0]](...args)
        this._api.push(match[1])
      }
    }
  }

  get api () {
    return this._api
  }

  get heapChar () {
    return this._module.HEAP8
  }

  get heapInt () {
    return this._module.HEAP32
  }

  get heapDouble () {
    return this._module.HEAPF64
  }
}

module.exports = {
  WASMDriver: WASMDriver
}
