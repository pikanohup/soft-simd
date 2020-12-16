const Benchmark = require('benchmark')
const { WASMDriver } = require('./lib/wasm-driver')
const { Task, PuppeteerPool } = require('./lib/puppeteer-pool')
const { DOUBLE_SIZE } = require('./lib/constants')

const log = (message) => {
  console.log(message)
}

const addBaselineCase = async (suite, page) => {
  const driver = await new WASMDriver('baseline-api')

  await page.exposeFunction('addArray', (a, n) => {
    return driver.addArray(a, n)
  })
  suite.add('Addition', {
    'defer': true,
    'fn': async (deferred) => {
      const n = 4
      const ptr = driver.malloc(n * DOUBLE_SIZE)
      for (let i = 0; i < n; i++) {
        driver.heapDouble[ptr / DOUBLE_SIZE + i] = i * 1.11
      }

      // ugly but effective :(
      const result = await page.evaluate(async (ptr, n) => {
        const result = await window.addArray(ptr, n)
        return result
      }, ptr, n)
      log(`test result is ${result}`)

      driver.free(ptr)
      deferred.resolve()
    }
  })
}

const addSoftsimdCase = async (suite, page) => {
  const driver = await new WASMDriver('softsimd-api')
  await page.exposeFunction('add', (a, b) => {
    return driver.add(a, b)
  })
  suite.add('Addition', {
    'defer': true,
    'fn': async (deferred) => {
      await page.evaluate(async () => {
        const a = 6
        const b = 6
        const result = await window.add(a, b)
        console.log(`test result of ${a} + ${b} is ${result}`)
      })
      deferred.resolve()
    }
  })
}

const perfTask = new Task(async (browser, suite, addCase) => {
  const page = await browser.newPage()
  await addCase(suite, page)
})

;(async () => {
  const puppeteerPool = await new PuppeteerPool()
  // eslint-disable-next-line new-parens
  const suite = new Benchmark.Suite

  await puppeteerPool.queue(perfTask, suite, addBaselineCase)

  suite.on('cycle', (event) => {
    log(`=== ${event.target.name} ===`)
    log('elapsed time:' + String(event.target.times.elapsed * 1000) + ' ms')
    log('mean time:' + String(event.target.stats.mean * 1000) + ' ms')
    log('stddev time:' + String(event.target.stats.deviation * 1000) + ' ms')
    log(String(event.target))
  })
    .on('error', (event) => {
      log(`test case ${event.target.name} failed`)
    })
    .on('complete', async (event) => {
      log(`\n -----------------------------------`)
      log(`Finished testing ${event.currentTarget.length} cases \n`)
      await puppeteerPool.clear()
    })
  suite.run({
    'async': true
  })
})()
