//! Short interpolation for discrete Mac wheel ticks, without an unbounded event queue.
use std::time::{Duration,Instant};
const STEP:Duration=Duration::from_millis(8);
const TAIL:Duration=Duration::from_millis(72);
const LIMIT:i64=3840;

#[derive(Default)]
pub(super) struct Scroll {pending:(i64,i64),last:Option<Instant>,end:Option<Instant>}
impl Scroll {
    pub fn cancel(&mut self){*self=Self::default();}
    pub fn push(&mut self,x:i32,y:i32,now:Instant)->Option<(i32,i32)>{
        if x==0&&y==0{return None}
        // A direction change supersedes the unfinished movement on that axis.
        if x!=0&&self.pending.0.signum()!=i64::from(x).signum(){self.pending.0=0;}
        if y!=0&&self.pending.1.signum()!=i64::from(y).signum(){self.pending.1=0;}
        let due=self.poll(now).unwrap_or((0,0));
        let total=(self.pending.0+i64::from(x),self.pending.1+i64::from(y));
        if total.0.abs()>LIMIT||total.1.abs()>LIMIT {
            self.cancel();return nonzero((due.0+total.0 as i32,due.1+total.1 as i32));
        }
        // Send a first step immediately, then finish within another 72 ms.
        let first=(x/10,y/10);
        self.pending=(total.0-i64::from(first.0),total.1-i64::from(first.1));
        self.last=Some(now);self.end=Some(now+TAIL);
        nonzero((due.0+first.0,due.1+first.1))
    }
    pub fn poll(&mut self,now:Instant)->Option<(i32,i32)>{
        let(last,end)=(self.last?,self.end?);
        if now>=end {let value=self.pending;self.cancel();return nonzero((value.0 as i32,value.1 as i32))}
        let elapsed=now.saturating_duration_since(last);
        if elapsed<STEP{return None}
        let ratio=elapsed.as_secs_f64()/end.duration_since(last).as_secs_f64();
        let value=((self.pending.0 as f64*ratio).round() as i32,(self.pending.1 as f64*ratio).round() as i32);
        self.pending.0-=i64::from(value.0);self.pending.1-=i64::from(value.1);self.last=Some(now);
        nonzero(value)
    }
}
fn nonzero(value:(i32,i32))->Option<(i32,i32)>{(value!=(0,0)).then_some(value)}

#[cfg(test)] mod tests {
    use super::*;
    #[test] fn one_tick_is_prompt_and_finishes_with_exact_distance(){
        for (x,y) in [(0,120),(0,-120),(120,0),(-240,360)] {
            let mut s=Scroll::default();let now=Instant::now();let first=s.push(x,y,now).unwrap();
            assert_eq!(first,(x/10,y/10));let mut total=first;
            for ms in (8..=72).step_by(8){let p=s.poll(now+Duration::from_millis(ms)).unwrap();total.0+=p.0;total.1+=p.1;}
            assert_eq!(total,(x,y));assert!(s.poll(now+Duration::from_secs(1)).is_none());
        }
    }
    #[test] fn rapid_ticks_keep_distance_and_have_a_bounded_tail(){
        for spacing in [0,1,8,16,40] {
            let mut s=Scroll::default();let start=Instant::now();let mut total=0;
            for n in 0..100 {if let Some(v)=s.push(0,120,start+Duration::from_millis(n*spacing)){total+=v.1;}}
            let finish=start+Duration::from_millis(99*spacing+72);
            if let Some(v)=s.poll(finish){total+=v.1;}
            assert_eq!(total,12000);assert!(s.poll(finish+Duration::from_secs(1)).is_none());
        }
    }
    #[test] fn reversal_and_cancellation_never_replay_old_direction(){
        let now=Instant::now();let mut s=Scroll::default();assert_eq!(s.push(0,120,now),Some((0,12)));
        assert_eq!(s.push(0,-120,now+STEP),Some((0,-12)));
        assert_eq!(s.poll(now+STEP+TAIL),Some((0,-108)));
        s.push(0,120,now);s.cancel();assert!(s.poll(now+Duration::from_secs(1)).is_none());
    }
    #[test] fn a_stalled_tick_catches_up_once_instead_of_queueing_frames(){
        let now=Instant::now();let mut s=Scroll::default();s.push(0,120,now);
        assert!(s.poll(now+Duration::from_millis(1)).is_none());
        assert_eq!(s.poll(now+Duration::from_secs(2)),Some((0,108)));
        assert!(s.poll(now+Duration::from_secs(3)).is_none());
    }
}
