// Create Battery Chart
function createBatteryChart() {
    var chart = new Highcharts.Chart({
        time: {
            useUTC: false
        },
        chart: {
            renderTo: 'chart-battery',
            type: 'spline'
        },
        series: [
            {
                name: 'EXIDE'
            }
        ],
        title: {
            text: undefined
        },
        plotOptions: {
            line: {
                animation: false,
                dataLabels: {
                    enabled: true
                }
            }
        },
        xAxis: {
            type: 'datetime',
            dateTimeLabelFormats: { second: '%H:%M:%S' }
        },
        yAxis: {
            title: {
                text: 'Battery Voltage Value'
            }
        },
        credits: {
            enabled: false
        }
    });
    return chart;
}

// Create Pump Chart
function createPumpChart() {
    var chart = new Highcharts.Chart({
        time: {
            useUTC: false
        },
        chart: {
            renderTo: 'chart-pump',
            type: 'spline'
        },
        series: [{
            name: 'Nouva Rade'
        }],
        title: {
            text: undefined
        },
        plotOptions: {
            line: {
                animation: false,
                dataLabels: {
                    enabled: true
                }
            },
            series: {
                color: '#50b8b4'
            }
        },
        xAxis: {
            type: 'datetime',
            dateTimeLabelFormats: { second: '%H:%M:%S' }
        },
        yAxis: {
            title: {
                text: 'Pump Voltage Value'
            }
        },
        credits: {
            enabled: false
        }
    });
    return chart;
}

// Create  Alarm Chart
function createAlarmChart() {
    var chart = new Highcharts.Chart({
        time: {
            useUTC: false
        },
        chart: {
            renderTo: 'chart-alarm',
            type: 'spline'
        },
        series: [{
            name: 'gmail'
        }],
        title: {
            text: undefined
        },
        plotOptions: {
            line: {
                animation: false,
                dataLabels: {
                    enabled: true
                }
            },
            series: {
                color: '#A62639'
            }
        },
        xAxis: {
            type: 'datetime',
            dateTimeLabelFormats: { second: '%H:%M:%S' }
        },
        yAxis: {
            title: {
                text: 'Number of calls'
            }
        },
        credits: {
            enabled: false
        }
    });
    return chart;
}

// Create Pressure Chart
function createCOChart() {
    var chart = new Highcharts.Chart({
        time: {
            useUTC: false
        },
        chart: {
            renderTo: 'chart-co',
            type: 'spline'
        },
        series: [{
            name: 'MQ7'
        }],
        title: {
            text: undefined
        },
        plotOptions: {
            line: {
                animation: false,
                dataLabels: {
                    enabled: true
                }
            },
            series: {
                color: '#A62639'
            }
        },
        xAxis: {
            type: 'datetime',
            dateTimeLabelFormats: { second: '%H:%M:%S' }
        },
        yAxis: {
            title: {
                text: 'Concentration (PPM)'
            }
        },
        credits: {
            enabled: false
        }
    });
    return chart;
}

// Create Pressure Chart
function createCH4Chart() {
    var chart = new Highcharts.Chart({
        time: {
            useUTC: false
        },
        chart: {
            renderTo: 'chart-ch4',
            type: 'spline'
        },
        series: [{
            name: 'MQ6'
        }],
        title: {
            text: undefined
        },
        plotOptions: {
            line: {
                animation: false,
                dataLabels: {
                    enabled: true
                }
            },
            series: {
                color: '#A62639'
            }
        },
        xAxis: {
            type: 'datetime',
            dateTimeLabelFormats: { second: '%H:%M:%S' }
        },
        yAxis: {
            title: {
                text: 'Concentration (PPM)'
            }
        },
        credits: {
            enabled: false
        }
    });
    return chart;
}

