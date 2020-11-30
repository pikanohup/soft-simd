const Benchmark = require('benchmark')
const puppeteer = require('puppeteer')
const { WASMDriver } = require('./lib/wasm-driver')

const log = (message) => {
  console.log(message)
}

const addAdditionCase = async (suite, page) => {
  const driver = await new WASMDriver('baseline')
  await page.exposeFunction('myTest', (a, b) => {
    return driver.add(a, b)
  })
  suite.add('Addition', {
    'defer': true,
    'fn': async (deferred) => {
      await page.evaluate(async () => {
        const a = 6
        const b = 6
        const result = await window.myTest(a, b)
        console.log(`test result of ${a} + ${b} is ${result}`)
      })
      deferred.resolve()
    }
  })
}

;(async () => {
  const browser = await puppeteer.launch({
    headless: true,
    defaultViewport: {
      width: 1366,
      height: 768,
      isMobile: false
    },
    args: [
      '--no-sandbox',
      '--disable-gpu'
    ]
  })
  const page = await browser.newPage()

  // eslint-disable-next-line new-parens
  const suite = new Benchmark.Suite

  await addAdditionCase(suite, page)

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
      await page.close()
      await browser.close()
    })
  suite.run({
    'async': true
  })
})()
