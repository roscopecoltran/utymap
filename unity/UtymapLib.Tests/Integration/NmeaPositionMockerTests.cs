using System;
using System.IO;
using System.Threading;
using Assets.UtymapLib.Core.Positioning;
using Assets.UtymapLib.Core.Positioning.Nmea;
using Assets.UtymapLib.Infrastructure;
using Assets.UtymapLib.Infrastructure.Reactive;
using NUnit.Framework;
using UtymapLib.Tests.Helpers;
using UtyRx;

namespace UtymapLib.Tests.Integration
{
    [TestFixture(Category = TestHelper.IntegrationTestCategory)]
    public class NmeaPositionMockerTests
    {
        [Test(Description = "Checks whether nmea file can be read.")]
        public void CanListenPositionChangeEvents()
        {
            // ARRANGE
            var waitEvent = new ManualResetEvent(false);
            var messageBus = new MessageBus();
            NmeaPositionMocker mocker;
            int count = 0;
            messageBus.AsObservable<GeoPosition>()
                .ObserveOn(Scheduler.ThreadPool)
                .Subscribe(position => count++);

            // ACT
            Action<TimeSpan> delayAction = _ => Thread.Sleep(0); // do not wait to make test faster
            using (Stream stream = new FileStream(TestHelper.NmeaFilePath, FileMode.Open))
            {
                mocker = new NmeaPositionMocker(stream, messageBus);
                mocker.OnDone += (s, e) => waitEvent.Set();
                mocker.Start(delayAction);
            }

            // ASSERT
            if(!waitEvent.WaitOne(TimeSpan.FromSeconds(60)))
                throw new TimeoutException();
            mocker.Stop();
            Assert.AreEqual(16, count);
        }
    }
}
