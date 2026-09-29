use kernel::{
    device::{
        self,
        Core, //
    },
    platform,
    prelude::*,
};

struct Cv1800b8051RprocDriver;

impl platform::Driver for Cv1800b8051RprocDriver {
    type IdInfo = ();
    type Data<'bound> = Self;

    fn probe<'bound>(
        pdev: &'bound platform::Device<Core<'_>>,
        info: Option<&'bound Self::IdInfo>,
    ) -> impl PinInit<Self, Error> + 'bound {
        let dev = pdev.as_ref();

        dev_info!(dev, "Cv1800b8051RprocDriver probed!");
        Ok(Self {})
    }
}

kernel::module_platform_driver! {
    type: Cv1800b8051RprocDriver,
    name: "cv1800-8051",
    authors: ["Andrei Lalaev <andrey.lalaev@gmail.com>"],
    description: "Sophgo CV1800 8051 coprocessor driver",
    license: "GPL v2",
}
