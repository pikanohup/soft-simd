const genericPool = require('generic-pool')
const puppeteer = require('puppeteer')
const { AsyncClass } = require('./async-class')

const { DEFAULT_TASK_TIMEOUT, DEFAULT_PUPPETEER_POOL_OPTION } = require('./constants')

class Task {
  constructor (callback, timeout = DEFAULT_TASK_TIMEOUT) {
    this._callback = callback
    this.timeout = timeout

    this._terminatePromise = new Promise((resolve, reject) => {
      this._terminateCallback = resolve
    })
  }

  async run (runner, ...args) {
    const timeoutPromise = new Promise((resolve, reject) => {
      setTimeout(resolve.bind(null, new Error('Timeout')), this.timeout)
    })
    try {
      return await Promise.race([
        Promise.resolve()
          .then(this._callback.bind(null, runner, ...args))
          .then(() => null)
          .catch(error => error),
        timeoutPromise,
        this._terminatePromise
      ])
    } catch (error) {
      return error
    }
  }

  terminate () {
    this._terminateCallback(new Error('Terminated'))
  }
}

class PuppeteerPool extends AsyncClass {
  constructor (options = DEFAULT_PUPPETEER_POOL_OPTION) {
    super(async () => {
      const { maxUseCount, browserOption } = options
      const factory = {
        create: async () => {
          const instance = await puppeteer.launch(browserOption)
          instance.useCount = 0
          return instance
        },
        destroy: (instance) => {
          return instance.close()
        },
        validate: (instance) => {
          return Promise.resolve(instance.useCount < maxUseCount)
        }
      }
      delete options.maxUseCount
      delete options.browserOption
      const pool = await genericPool.createPool(factory, options)
      this._pool = pool
    })
  }

  async acquire () {
    const instance = await this._pool.acquire()
    instance.useCount++
    return instance
  }

  async queue (task, ...args) {
    let instance
    try {
      instance = await this.acquire()
      return await task.run(instance, ...args)
    } catch (error) {
      return error
    } finally {
      this._pool.release(instance)
    }
  }

  async clear () {
    await this._pool.drain()
    await this._pool.clear()
  }
}

module.exports = {
  Task: Task,
  PuppeteerPool: PuppeteerPool
}
