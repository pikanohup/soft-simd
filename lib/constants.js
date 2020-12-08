module.exports = {
  DEFAULT_TASK_TIMEOUT: 9000,
  DEFAULT_BROWSER_OPTION: {
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
  },
  DEBUG_BROWSER_OPTION: {
    headless: false,
    args: [
      '--no-sandbox',
      '--disable-gpu'
    ]
  },
  DEFAULT_PUPPETEER_POOL_OPTION: {
    max: 5,
    min: 2,
    maxUseCount: 1024,
    testOnBorrow: true,
    autoStart: false,
    idleTimeoutMillis: 1800000, // 30 min
    evictionRunIntervalMillis: 180000, // 3min
    browserOption: this.DEFAULT_BROWSER_OPTION
  },
  DEBUG_PUPPETEER_POOL_OPTION: {
    max: 2,
    min: 1,
    maxUseCount: 2,
    testOnBorrow: true,
    autoStart: false,
    idleTimeoutMillis: 1800000, // 30 min
    evictionRunIntervalMillis: 60000, // 1min
    browserOption: this.DEBUG_BROWSER_OPTION
  }
}

